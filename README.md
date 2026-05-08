# 🐧 Linux Container Runtime

A lightweight Linux container runtime built from scratch using **namespaces**, **seccomp**, and **cgroups** — without Docker or any existing container framework.

> Built as an Operating Systems project to understand how modern container technologies like Docker work internally.

---

## 📋 Features

| Feature | Mechanism | Description |
|---|---|---|
| Process Isolation | PID Namespace | Container processes cannot see host PIDs |
| Filesystem Isolation | Mount Namespace | Separate `/proc` — only container processes visible |
| Hostname Isolation | UTS Namespace | Container gets its own hostname (`mycontainer`) |
| Syscall Filtering | Seccomp | Blocks dangerous syscalls (`ptrace`, `reboot`, `kexec_load`, etc.) |
| Resource Limiting | Cgroups v2 | 256 MB memory cap, 50% CPU quota |

---

## 🗂️ Project Structure

```
linux-container-runtime/
├── src/
│   └── container.c          # Main C source — container runtime
├── notebooks/
│   └── container_runtime.ipynb   # Jupyter Notebook walkthrough
├── scripts/
│   ├── setup.sh             # Install deps + build
│   └── cleanup.sh           # Remove cgroup after run
├── Makefile                 # Build system
└── README.md
```

---

## ⚙️ Requirements

- **OS:** Linux (Kali Linux, Ubuntu 22.04+, Debian)
- **Privileges:** Root / sudo
- **Dependencies:** `gcc`, `libseccomp-dev`
- **Kernel:** Cgroups v2 enabled (default on modern distros)

> ⚠️ Does **not** work on Windows or macOS natively. Use VirtualBox/VMware with a Linux guest.

---

## 🚀 Quick Start

### Option A — Terminal

```bash
# 1. Clone the repo
git clone https://github.com/YOUR_USERNAME/linux-container-runtime.git
cd linux-container-runtime

# 2. Install dependencies & build
chmod +x scripts/setup.sh
./scripts/setup.sh

# 3. Run the container
sudo ./container
```

### Option B — Jupyter Notebook

```bash
# Install Jupyter (if not already)
pip install notebook

# Launch
jupyter notebook notebooks/container_runtime.ipynb
```

Run each cell top to bottom. The final cell performs a non-interactive test.

### Option C — Manual Build

```bash
sudo apt-get install -y gcc libseccomp-dev
gcc -Wall -O2 -o container src/container.c -lseccomp
sudo ./container
```

---

## ✅ Verifying Isolation

Once inside the container shell, run these commands:

```bash
# 1. PID Isolation — should print: 1
echo $$

# 2. Hostname Isolation — should print: mycontainer
hostname

# 3. Process Visibility — only container processes
ps aux

# 4. Cgroup membership
cat /proc/self/cgroup

# 5. Exit
exit
```

---

## 🏗️ Architecture

```
User
 └─► main()
      ├─► setup_cgroups()      — CPU + Memory limits via /sys/fs/cgroup
      └─► clone()              — CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS
           └─► child_func()
                ├─► sethostname()   — UTS namespace
                ├─► mount()         — Mount namespace (/proc remount)
                ├─► setup_seccomp() — Syscall filter
                └─► execv(/bin/bash) — Container shell
```

---

## 🔒 Seccomp Blocked Syscalls

| Syscall | Reason |
|---|---|
| `ptrace` | Prevent process tracing / debugging of host |
| `reboot` | Prevent host reboot from container |
| `kexec_load` | Prevent kernel replacement |
| `mount` | Prevent host filesystem manipulation |
| `umount2` | Prevent unmounting host mounts |
| `pivot_root` | Prevent root filesystem swap |
| `swapon/swapoff` | Prevent swap manipulation |

---

## ⚠️ Limitations

- No **network namespace** (no network isolation)
- No **image management** (uses host filesystem)
- Requires **root privileges**
- No container **orchestration**
- Cgroups v2 only (check with `stat -fc %T /sys/fs/cgroup/`)

---

## 🔮 Future Enhancements

- [ ] Network namespace (`CLONE_NEWNET`)
- [ ] Overlay filesystem support
- [ ] Docker-like CLI (`run`, `ps`, `stop`)
- [ ] User namespace (`CLONE_NEWUSER`) — rootless containers
- [ ] Container checkpoint & restore (CRIU)

---

## 📚 References

- [Linux `clone()` man page](https://man7.org/linux/man-pages/man2/clone.2.html)
- [Linux Namespaces](https://man7.org/linux/man-pages/man7/namespaces.7.html)
- [Seccomp — libseccomp](https://github.com/seccomp/libseccomp)
- [Control Groups v2 — kernel.org](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html)
- [Docker Internals](https://docs.docker.com/engine/reference/run/)

---

## 📄 License

MIT License — free to use, modify, and distribute.
