void apply_seccomp() {
    scmp_filter_ctx ctx;

    ctx = seccomp_init(SCMP_ACT_KILL); // default kill
    if (!ctx) {
        perror("seccomp_init failed");
        exit(1);
    }

    // Essential syscalls for basic programs
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0);

    // 🔥 ADD THESE (important for bash)
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(access), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(arch_prctl), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_tid_address), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_robust_list), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(prlimit64), 0);

    if (seccomp_load(ctx) < 0) {
        perror("seccomp_load failed");
        exit(1);
    }
}
