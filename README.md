# Linux Container Runtime (From Scratch)

This project demonstrates a lightweight container runtime built using low-level Linux features like namespaces and seccomp.

---

## 🔧 Technologies Used

- C Programming (GCC)
- Linux Namespaces:
  - PID Namespace (process isolation)
  - Mount Namespace (filesystem isolation)
  - UTS Namespace (hostname isolation)
- Seccomp (system call filtering)

---

## 🚀 Features

- ✅ Process isolation (PID namespace)
- ✅ Filesystem isolation with custom `/proc`
- ✅ Custom hostname inside container
- ✅ Basic syscall restriction using seccomp
- ✅ Minimal container runtime built from scratch

---

## 🧪 How It Works

The program uses the `clone()` system call to create a new process with isolated namespaces.

Inside the container:
- A new `/proc` filesystem is mounted
- A custom hostname is set
- A restricted syscall environment is applied (seccomp)

---

## ▶️ How to Run

```bash
gcc container.c -o container
sudo ./container
