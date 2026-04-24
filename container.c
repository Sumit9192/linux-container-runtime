#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>

#define STACK_SIZE (1024 * 1024)
static char stack[STACK_SIZE];

int child_func(void *arg) {
    printf("Inside container!\n");

    // Make mount private
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
        perror("mount private failed");
        exit(1);
    }

    // Mount new /proc
    if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
        perror("mount proc failed");
        exit(1);
    }

    execlp("/bin/bash", "bash", NULL);

    perror("execlp failed");
    return 1;
}

int main() {
    printf("Starting container...\n");

    pid_t pid = clone(child_func, stack + STACK_SIZE,
                      CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, NULL);

    if (pid == -1) {
        perror("clone failed");
        exit(1);
    }

    waitpid(pid, NULL, 0);
    printf("Container exited.\n");

    return 0;
}
