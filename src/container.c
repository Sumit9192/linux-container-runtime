/*
 * Linux Container Runtime
 * Using Namespaces, Seccomp, and Cgroups
 *
 * Compile: gcc -o container container.c -lseccomp
 * Run:     sudo ./container
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <seccomp.h>
#include <errno.h>

#define STACK_SIZE      (1024 * 1024)   /* 1 MB stack for child process */
#define CGROUP_PATH     "/sys/fs/cgroup/mycontainer"
#define HOSTNAME        "mycontainer"
#define MEM_LIMIT       "268435456"     /* 256 MB in bytes */
#define CPU_LIMIT       "50000 100000"  /* 50% CPU quota */

static char child_stack[STACK_SIZE];

/* ─────────────────────────────────────────────
   Helper: write a string to a file
   ───────────────────────────────────────────── */
static int write_file(const char *path, const char *value) {
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "[-] Cannot open %s: %s\n", path, strerror(errno));
        return -1;
    }
    fprintf(f, "%s\n", value);
    fclose(f);
    return 0;
}

/* ─────────────────────────────────────────────
   SECCOMP: Whitelist-based syscall filtering
   ───────────────────────────────────────────── */
static void setup_seccomp(void) {
    printf("[*] Applying seccomp filters...\n");

    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (!ctx) {
        perror("[-] seccomp_init");
        exit(EXIT_FAILURE);
    }

    /* Block dangerous syscalls */
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(ptrace),     0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(reboot),     0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(kexec_load), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(mount),      0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(umount2),    0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(pivot_root), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(swapon),     0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(swapoff),    0);

    if (seccomp_load(ctx) < 0) {
        perror("[-] seccomp_load");
        seccomp_release(ctx);
        exit(EXIT_FAILURE);
    }

    seccomp_release(ctx);
    printf("[+] Seccomp filters applied.\n");
}

/* ─────────────────────────────────────────────
   CGROUPS: Resource limits (CPU + Memory)
   ───────────────────────────────────────────── */
static void setup_cgroups(void) {
    printf("[*] Setting up cgroups at %s...\n", CGROUP_PATH);

    if (mkdir(CGROUP_PATH, 0755) < 0 && errno != EEXIST) {
        fprintf(stderr, "[-] mkdir cgroup: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Memory limit */
    char mem_path[256], cpu_path[256], procs_path[256];
    snprintf(mem_path,   sizeof(mem_path),   "%s/memory.max",    CGROUP_PATH);
    snprintf(cpu_path,   sizeof(cpu_path),   "%s/cpu.max",       CGROUP_PATH);
    snprintf(procs_path, sizeof(procs_path), "%s/cgroup.procs",  CGROUP_PATH);

    write_file(mem_path, MEM_LIMIT);
    write_file(cpu_path, CPU_LIMIT);

    /* Add this process to the cgroup */
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());
    write_file(procs_path, pid_str);

    printf("[+] Cgroups configured: CPU=%s  MEM=%s bytes\n", CPU_LIMIT, MEM_LIMIT);
}

/* ─────────────────────────────────────────────
   CHILD: Runs inside the container
   ───────────────────────────────────────────── */
static int child_func(void *arg) {
    (void)arg;

    /* UTS namespace: set hostname */
    printf("[*] Setting hostname to '%s'...\n", HOSTNAME);
    if (sethostname(HOSTNAME, strlen(HOSTNAME)) < 0) {
        perror("[-] sethostname");
        return EXIT_FAILURE;
    }
    printf("[+] Hostname set.\n");

    /* Mount namespace: make root private, remount /proc */
    printf("[*] Configuring mount namespace...\n");
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) < 0) {
        perror("[-] mount private root");
        return EXIT_FAILURE;
    }
    if (mount("proc", "/proc", "proc", 0, NULL) < 0) {
        perror("[-] mount proc");
        return EXIT_FAILURE;
    }
    printf("[+] Filesystem isolation done.\n");

    /* Seccomp filtering */
    setup_seccomp();

    /* Launch a bash shell inside the container */
    printf("\n[+] Container ready! Launching shell...\n");
    printf("    Hostname : %s\n", HOSTNAME);
    printf("    PID      : %d (should be 1 inside namespace)\n\n", getpid());

    char *args[] = {"/bin/bash", NULL};
    execv("/bin/bash", args);

    /* execv only returns on error */
    perror("[-] execv");
    return EXIT_FAILURE;
}

/* ─────────────────────────────────────────────
   MAIN: Entry point
   ───────────────────────────────────────────── */
int main(void) {
    if (getuid() != 0) {
        fprintf(stderr, "[-] This program must be run as root.\n");
        exit(EXIT_FAILURE);
    }

    printf("=== Linux Container Runtime ===\n\n");

    /* Step 1: Cgroups (must run before clone) */
    setup_cgroups();

    /* Step 2: Create child in new namespaces */
    printf("[*] Cloning child process with new namespaces...\n");
    pid_t pid = clone(
        child_func,
        child_stack + STACK_SIZE,
        CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | SIGCHLD,
        NULL
    );

    if (pid < 0) {
        perror("[-] clone");
        exit(EXIT_FAILURE);
    }

    printf("[+] Container started with host PID: %d\n\n", pid);

    /* Wait for container to exit */
    waitpid(pid, NULL, 0);

    printf("\n[+] Container exited cleanly.\n");
    return EXIT_SUCCESS;
}
