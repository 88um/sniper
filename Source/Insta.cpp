#include "Insta.h"



//Properties
std::map<std::string, std::string> Insta::headers() {
    std::map<std::string, std::string>  headers;
    headers["authority"] = "www.instagram.com";
    headers["accept"] = "*/*";
    headers["accept-language"] = "en-US,en;q=0.9";
    headers["content-type"] = "application/x-www-form-urlencoded";
    headers["origin"] = "https://www.instagram.com";
    headers["referer"] = "https://www.instagram.com/accounts/edit/";
    headers["sec-ch-prefers-color-scheme"] = "light";
    headers["sec-ch-ua"] = "\"Google Chrome\";v=\"111\", \"Not(A:Brand\";v=\"8\", \"Chromium\";v=\"111\"";
    headers["sec-ch-ua-mobile"] = "?0";
    headers["sec-ch-ua-platform"] = "\"Windows\"";
    headers["sec-fetch-dest"] = "empty";
    headers["sec-fetch-mode"] = "cors";
    headers["sec-fetch-site"] = "same-origin";
    headers["user-agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/111.0.0.0 Safari/537.36";
    headers["viewport-width"] = "786";
    headers["x-asbd-id"] = "198387";
    headers["x-ig-app-id"] = "936619743392459";
    headers["x-ig-www-claim"] = "hmac.AR35ZY1oKWXPBETIz1GpEnCQPS5uwDBHWhRCGmvFXDC9bmwl";
    headers["x-instagram-ajax"] = "1007164808";
    headers["x-requested-with"] = "XMLHttpRequest";
    if (!this->csrf.empty()) { headers["x-csrftoken"] = this->csrf; }
    return headers;
}


//Attributes


bool Insta::isClaimed() {
    return this->claimed;
}

bool Insta::isLoggedIn() {
    return this->is_logged_in;
}

int Insta::lastCode() {
    return this->last_response.status_code;
}

std::string Insta::lastText() {
    return this->last_response.text;
}

Response Insta::lastResponse() {
    return this->last_response;
}

std::string Insta::userId() {
    std::string substr = "";
    std::size_t pos = this->session.find("%");

    if (pos != std::string::npos) {
        substr = this->session.substr(0, pos);
    }
    return substr;
}



//Maain funcs

void Insta::setSession(const std::string& session) {
    this->session = session;
    this->getData();
    this->requests.cookies["sessionid"] = session;
    this->requests.cookies["ds_user_id"] = this->userId();
}

bool Insta::sendRequest(const std::string& url, const std::string& data) {
    Response response;
    std::string proxy;
    std::map<std::string, std::string> headers = this->headers();
    this->requests.cookies.erase("csrftoken"); //Dont fix what aint broke
    if (this->use_proxies) { proxy = random_proxy();}
    if (data.empty()) {
        response = this->requests.get(url, headers, proxy);
        while (response.status_code == 0) {
            response = this->requests.get(url, headers, proxy);
        };
        
    }
    else {
        response = this->requests.post(url, headers, data, proxy);
        while (response.status_code == 0) {
            response = this->requests.post(url, headers, data, proxy);
        };
    }
    this->last_response = response;
    this->handleErrors();
    return (response.status_code == 200 && !response.json.is_null());
}


void Insta::claimUser(const std::string& username) {
    std::string url = "https://www.instagram.com/api/v1/web/accounts/edit/";
    std::map<std::string, std::string> data;
    data["username"] = username;
    data["first_name"] = this->full_name;
    data["email"] = this->email;
    data["phone_number"] = this->phone;
    data["biography"] = this->bio;
    data["external_url"] = this->external_url;
    data["chaining_enabled"] = "on";
    if (this->sendRequest(url,sign(data))) {
        this->claimed = true;
        this->target = username;
    }
    else {
        for (const auto& pair : this->headers()) {
            std::cout<< pair.first + ": " + pair.second + "; \n";
        }
    }
   
}

bool Insta::profileInfo() {
    std::string url = "https://www.instagram.com/api/v1/accounts/edit/web_form_data/";
    if (this->sendRequest(url)) {

        this->full_name = this->last_response.json["form_data"]["first_name"];
        this->email = this->last_response.json["form_data"]["email"];
        this->username = this->last_response.json["form_data"]["username"];
        this->phone = this->last_response.json["form_data"]["phone_number"];
        this->external_url = this->last_response.json["form_data"]["external_url"];
        return true;
    }
    else {
        return false;
    }
    
}


void Insta::getData() {
    this->sendRequest("https://www.instagram.com/data/shared_data/");
    this->csrf = this->last_response.json["config"]["csrf_token"];
    this->requests.cookies["csrftoken"] = this->csrf;
}

bool Insta::login(const std::string& username, const std::string& password) {
    this->username = username;
    this->password = password;
    this->getData();
    std::time_t t = std::time(nullptr);
    std::string str_time = std::to_string(t);
    std::string new_password = "#PWD_INSTAGRAM_BROWSER:0:" + str_time + ":" + password;
    std::map<std::string, std::string> data = {
        {"username", username},
        {"enc_password", new_password}
    };
    this->sendRequest("https://www.instagram.com/accounts/login/ajax/", sign(data));
    this->handleChallenge();
    this->is_logged_in = true;
    return this->lastCode()==200;
}



//Utils
void Insta::handleErrors() {
    std::size_t pos = this->last_response.text.find("/suspended");
    std::size_t rated = this->last_response.text.find("generic_request_error");
    std::size_t wrong_pass = this->last_response.text.find(" your password was incorrect");
    std::size_t challenge = this->last_response.text.find("challenge_required");
    if (challenge != std::string::npos) {
        throw LoginException("[!] Your account is challenge locked. Please complete the challenge and retry logging in...");
    }
    if (wrong_pass != std::string::npos) {
        throw LoginException("[!] Incorrect username/password credentials supplied...");
    }
    else if (rated != std::string::npos) {
        throw LoginException("[!] You are rate-limited from logging in! Please login by session or enable proxies...");
    }
    else if (this->last_response.status_code == 403 || pos != std::string::npos) {
        this->removeSelf();
    }

}


bool Insta::sendCode(const std::string& option) {
    std::map < std::string, std::string> data;
    std::cout <<"Challenge uRL: " << this->chal_url + "\n";

    data["choice"] = option;
    this->sendRequest(this->chal_url, sign(data));
    return (this->lastCode() == 200 && !this->last_response.json.is_null());

}


bool Insta::enterCode(const std::string& code) {
    std::map < std::string, std::string> data;
    data["security_code"] = code;
    this->sendRequest(this->chal_url, sign(data));
    return (this->lastCode() == 200 && !this->last_response.json.is_null());

}

void Insta::handleChallenge() {
    std::string option;
    std::size_t checkpoint = this->last_response.text.find("checkpoint_required");
    if (checkpoint != std::string::npos) {
        std::cout << redText("[!] Challenge code required!\n");
        std::cout << "[0] Send code to phone\n";
        std::cout << "[1] Send code to email\n";
        std::cout << "[+] Choose One: ";
        std::cin >> option;
        std::string checkpoint = this->last_response.json["checkpoint_url"];
        this->chal_url = "https://www.instagram.com" + checkpoint;
        if (!this->sendCode(option)) {throw LoginException("[!] Failed to send security code to choice entered...");}
        std::cout << "[+] Enter your 6 digit code: ";
        std::cin >> option;
        if (!this->enterCode(option)) { throw LoginException("[!] Incorrect security code supplied..."); }
    }
}


void Insta::removeSelf() {
    std::lock_guard<std::mutex> lockGuard(file_lock);
    if (this->claimed) {
        std::ofstream file("claims.txt", std::ios::app);
        if (file.is_open()) {
            file << this->target + ": " + this->session + "\n";
            file.close();
        }

    }
    std::fstream usersFile("users.txt", std::ios::in | std::ios::out);
    if (!usersFile.is_open()) {
        return;
    }


    std::fstream sessionsFile("sessions.txt", std::ios::in | std::ios::out);
    if (!sessionsFile.is_open()) {
        return;
    }


    std::string line;
    std::ofstream tempFile("temp.txt");
    while (std::getline(sessionsFile, line)) {
        if (line != this->session) {
            tempFile << line << std::endl;
        }
    }
    sessionsFile.close();
    tempFile.close();
    std::remove("sessions.txt");
    std::rename("temp.txt", "sessions.txt");


    std::ofstream tempFile2("temp.txt");
    while (std::getline(usersFile, line)) {
        if (line != this->target) {
            tempFile2 << line << std::endl;
        }
    }
    usersFile.close();
    tempFile2.close();
    std::remove("users.txt");
    std::rename("temp.txt", "users.txt");

}

