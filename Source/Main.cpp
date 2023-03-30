
#include "Insta.h"

std::map<std::string, Insta> infomap; 
std::set<std::string> errorMessages;


void successMessage(const std::string& target) {
    std::string text = "@" + target + " has been successfully claimed.";
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
    std::vector<wchar_t> buffer(len);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buffer.data(), len);
    MessageBox(NULL, buffer.data(), TEXT("New Slimy Claim!!"), MB_ICONINFORMATION | MB_OK);
}


void swap(Insta& ig,  std::string& target) {
    while (!ig.isClaimed()) {
        ig.claimUser(target);
        if (ig.lastCode() == 429 || ig.lastCode() == 403) {
            break;
        }
    } 
}

void claim() {
    while (true) {
        try {
            Insta ig;
            std::string session = get_session();
            std::string target = get_user();
            if (infomap.count(session)) {
                ig = infomap[session];
            }
            else {
                ig.setSession(session);
                if (!ig.profileInfo()) { ig.removeSelf(); continue; };
                infomap[session] = ig;
            }
            ig.claimUser(target);
            if (!ig.isClaimed()) {
                std::lock_guard<std::mutex> lock(print_lock);
                std::cout << redText("[-] Failed to claim user: " + target + " | Response: " + std::to_string(ig.lastCode())) << "\n";
            }
            else {
                std::lock_guard<std::mutex> lock(print_lock);
                std::cout << greenText("[+] Successfully claimed user: " + target) << "\n";
                Discord::send(target);
                std::thread t(successMessage, target);
                t.detach();
                ig.removeSelf();
                break;

            }
          }
        catch (std::exception& e) {
            std::lock_guard<std::mutex> lock(print_lock);
            if (errorMessages.find(e.what()) == errorMessages.end()) {
                errorMessages.insert(e.what());
                std::cout << e.what();

            }   
            break;
        }
    }
   
    
}

void start(int& num_threads, std::string& target, Insta& ig ) {
    std::vector<std::thread> threads;
    std::thread checker(track_requests);
    checker.detach();
    for (int i = 0; i < num_threads; i++) {
        if (!ig.isLoggedIn()) {
            threads.push_back(std::thread(claim));
        }
        else {
            threads.push_back(std::thread(swap, std::ref(ig), std::ref(target)));
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
   

}


void claimer() {
    Insta ig;
    std::string target;
    std::string username;
    std::string password;
    std::string use_proxies;
    int choice;
    int num_threads;
    clear_screen();
    std::cout << "[1] Swapper\n[2] Auto-Claimer\n\n[+] Choose One: ";
    std::cin >> choice;
    clear_screen();
    if (choice == 2) {
        try {
            load();
            std::cout << "[-] Loaded " << users.size() << " target users and " << sessions.size() << " sessions!\n";
            std::cout << "[+] Enter the number of threads to create: ";
            std::cin >> num_threads;
            start(num_threads, target, ig);
        }
        catch (std::exception& e) { std::cout << e.what(); }
    }
    else {
        try {
            std::cout << yellowText("[+] Enter Your Username: ");
            std::cin >> username;
            std::cout << yellowText("[+] Enter Your Password: ");
            std::cin >> password;

            ig.login(username, password);
            if (!ig.profileInfo()) { throw LoginException("[!] Failed to gather account info | Response: " + std::to_string(ig.lastCode())); }


            std::cout << greenText("[SUCCESS] ") << "Logged into user >> " << username << "\n";
            std::cout << magentaText("[+]") << " Enter Target Username : ";
            std::cin >> target;
            std::cout << magentaText("[+]") << " Enter Amount of Threads : ";
            std::cin >> num_threads;
            std::cout << magentaText("[+]") << " Use HTTP Proxies (Y/N) : ";
            std::cin >> use_proxies;
            std::transform(use_proxies.begin(), use_proxies.end(), use_proxies.begin(),
                [](unsigned char c) { return std::tolower(c); });
            if (use_proxies.compare("n") == 0) { ig.use_proxies = false; }
            MessageBox(nullptr, TEXT("Press OK when you are ready to start!"), TEXT("Slime Claimer"), MB_OK);
            start(num_threads, target, ig);
            if (!ig.isClaimed()) { std::cout << "\033[1;31m" << "[-] Failed to claim user: " << target << " | Response: " << ig.lastCode() << "\033[0m" << "\n"; }
            else {
                std::cout << "\033[1;32m" << "[+] Successfully claimed user: " << target << "\033[0m" << "\n"; successMessage(target);
            }
            std::cout << total_requests;

        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

    }
}





int main() {
    
    claimer();
    std::puts("\nPress any key to exit...");
    getchar();
    
    return 0;
}

