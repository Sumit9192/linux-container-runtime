#!/bin/bash
# ─────────────────────────────────────────────────────────────
#  cleanup.sh — Remove cgroup created by the container runtime
# ─────────────────────────────────────────────────────────────

CGROUP_PATH="/sys/fs/cgroup/mycontainer"

echo "[*] Cleaning up cgroup: $CGROUP_PATH"

if [ -d "$CGROUP_PATH" ]; then
    sudo rmdir "$CGROUP_PATH" 2>/dev/null && \
        echo "[+] Cgroup removed." || \
        echo "[!] Could not remove cgroup (processes may still be running)."
else
    echo "[+] Cgroup not found — nothing to clean."
fi

echo "[+] Cleanup done."
