# ─────────────────────────────────────────────────────────────
#  Makefile — Linux Container Runtime
# ─────────────────────────────────────────────────────────────

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -D_GNU_SOURCE
LIBS    = -lseccomp
SRC     = src/container.c
TARGET  = container

.PHONY: all clean run install-deps

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)
	@echo "[+] Build successful: ./$(TARGET)"

install-deps:
	@echo "[*] Installing dependencies..."
	sudo apt-get update
	sudo apt-get install -y gcc libseccomp-dev
	@echo "[+] Dependencies installed."

run: $(TARGET)
	@echo "[*] Running container (requires root)..."
	sudo ./$(TARGET)

clean:
	rm -f $(TARGET)
	@echo "[+] Cleaned build artifacts."
