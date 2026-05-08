#!/bin/bash
# ─────────────────────────────────────────────────────────────
#  setup.sh — Install dependencies and build the container runtime
# ─────────────────────────────────────────────────────────────

set -e

echo "=== Linux Container Runtime — Setup ==="
echo ""

# Check Linux
if [[ "$(uname -s)" != "Linux" ]]; then
    echo "[-] This project requires Linux. Exiting."
    exit 1
fi

# Install dependencies
echo "[*] Installing dependencies..."
sudo apt-get update -qq
sudo apt-get install -y gcc libseccomp-dev make
echo "[+] Dependencies installed."

# Build
echo "[*] Building..."
make clean
make
echo "[+] Build complete."

echo ""
echo "=== Setup Complete ==="
echo "Run the container with:  sudo ./container"
echo "Or use make:             make run"
