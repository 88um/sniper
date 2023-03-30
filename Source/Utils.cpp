#include "Utils.h"


#ifdef _WIN32
#define CLEAR_COMMAND "cls"
const std::string console_title_sequence = "\033]0;";
const std::string console_title_suffix = "\007";
#else
const std::string console_title_sequence = "\033]2;";
const std::string console_title_suffix = "\033\\";
#define CLEAR_COMMAND "clear"
#endif


std::mutex proxy_lock;
std::mutex print_lock;
std::mutex file_lock;
std::mutex users_lock;
std::mutex sessions_lock;
std::queue<std::string> users;
std::queue<std::string> sessions;
int total_requests = 0;
int total_errors = 0;
std::atomic<int> requestCount = 0;
enum Color { RED, GREEN, BLUE, YELLOW, PURPLE };
std::vector<int> color_values = { 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0x800080 };


std::string logo =
"    .--.--.     ,--,                      ____           \n"
"   /  /    '. ,--.'|     ,--,           ,'  , `.         \n"
"  |  :  /`. / |  | :   ,--.'|        ,-+-,.' _ |         \n"
"  ;  |  |--`  :  : '   |  |,      ,-+-. ;   , ||         \n"
"  |  :  ;_    |  ' |   `--'_     ,--.'|'   |  || ,---.   \n"
"   \\  \\    `. '  | |   ,' ,'|   |   |  ,', |  |,/     \\  \n"
"    `----.   \\|  | :   '  | |   |   | /  | |--'/    /  | \n"
"    __ \\  \\  |'  : |__ |  | :   |   : |  | ,  .    ' / | \n"
"   /  /`--'  /|  | '.'|'  : |__ |   : |  |/   '   ;   /| \n"
"  '--'.     / ;  :    ;|  | '.'||   | |`-'    '   |  / | \n"
"    `--'---'  |  ,   / ;  :    ;|   ;/        |   :    | \n"
"               ---`-'  |  ,   / '---'          \\   \\  /  \n"
"                        ---`-'                  `----'   \n\n"
"                free claimer/swapper by joshua (@ulzi) \n\n";



int get_random_color() {
    srand(time(0));
    int index = rand() % color_values.size();
    return color_values[static_cast<Color>(index)];
}

bool file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

bool load() {
    users = queue_file("users.txt");
    sessions = queue_file("sessions.txt");
    return (!users.empty() && !sessions.empty());
}

void clear_screen() {
    system(CLEAR_COMMAND);
    std::cout << "\033[1;33m" << logo << "\033[0m";
}

void track_requests() {
    while (true) {
        int count = requestCount.exchange(0);
        double rps = static_cast<double>(count) / 1.0; 
        int n_rps = static_cast<int>(rps);
        std::string console_title = console_title_sequence + "Attempts: " + std::to_string(total_requests) + " | "+ "Errors: " + std::to_string(total_errors) +" | " + "R/S: " +std::to_string(n_rps) + console_title_suffix;
        std::cout << console_title;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


std::string get_user() {
    std::lock_guard<std::mutex> lockGuard(users_lock);
    if (users.empty()) {
        std::queue<std::string> userz;
        userz = queue_file("users.txt");
        users = std::queue<std::string>();
        users = userz;
    }
    std::string user = users.front();
    users.pop();
    return (user);
}

std::string get_session() {
    std::lock_guard<std::mutex> lockGuard(sessions_lock);
    if (sessions.empty()) {
        std::queue<std::string> sessionz;
        sessionz = queue_file("sessions.txt");
        sessions = std::queue<std::string>();
        sessions = sessionz;
    }
    std::string session = sessions.front();
    sessions.pop();
    return (session);
}


std::string cookie_header(std::map<std::string, std::string>& cookies) {
    std::string cookie_header;
    for (const auto& pair : cookies) {
        cookie_header += pair.first + "=" + pair.second + "; ";
    }
    cookie_header = cookie_header.substr(0, cookie_header.length() - 2);
    return cookie_header;
}


std::string url_encode(const std::string& str) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (auto c : str) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        }
        else {
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int((unsigned char)c);
            escaped << std::nouppercase;
        }
    }

    return escaped.str();
}


std::string sign(const std::map<std::string, std::string>& my_map) {
    std::string query_string;
    for (auto it = my_map.begin(); it != my_map.end(); ++it) {
        query_string += it->first + "=" + url_encode(it->second) + "&";
    }
    if (!query_string.empty()) {
        query_string.pop_back();
    }

    return query_string;
}

std::string random_proxy() {

    std::lock_guard<std::mutex> lockGuard(proxy_lock);
    std::ifstream file("proxies.txt");
    if (!file) {return "";}

   
    std::vector<std::string> proxies;
    std::string line;
    while (std::getline(file, line)) {
        proxies.push_back(line);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, proxies.size() - 1);
    int index = dis(gen);
    return proxies[index];
}


std::string mask_pass() {
    char password[50];
    int i = 0;
    char ch;

    while (true) {
        ch = _getch();

        if (ch == 13) {
            std::cout << std::endl;
            break;
        }
        else if (ch == 8) { 
            if (i > 0) {
                i--;
                std::cout << "\b \b";
            }
        }
        else {
            password[i] = ch;
            i++;
            std::cout << "*";
        }
    }

    password[i] = '\0';

    return std::string(password);
}


std::queue<std::string> queue_file(std::string filename) {
    std::lock_guard<std::mutex> lockGuard(file_lock);
    if (!file_exists(filename)) {
        std::ofstream file(filename);
    }
    std::ifstream inputFile(filename);
    std::string line;
    std::queue<std::string> linesQueue;
    while (std::getline(inputFile, line)) {
        linesQueue.push(line);
    }
    inputFile.close();
    if (linesQueue.empty()) { throw EmptyFileException("[!] " + filename + " is empty! Please add info to the file and try again..."); }
    return linesQueue;
}

ProxyInfo parse_proxy(const std::string& address) {
    ProxyInfo info;
    size_t atPos = address.find('@');
    size_t colonPos = address.find(':');
    if (atPos != std::string::npos && colonPos != std::string::npos && colonPos < atPos) {
        info.username = address.substr(0, colonPos);
        info.password = address.substr(colonPos + 1, atPos - colonPos - 1);
        info.proxy = "http://"+ address.substr(atPos + 1);
        info.user_pass = info.username + ":" + info.password;
    }
    else {
        info.username = "";
        info.password = "";
        info.proxy = "http://" + address;
    }
    return info;
}
