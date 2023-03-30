#include "Requests.h"


size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    static_cast<std::string*>(userdata)->append((char*)ptr, size * nmemb);
    return nmemb;
}

size_t header_callback(char* buffer, size_t size, size_t nitems, std::map<std::string, std::string>& cookies) {
    const std::string header(buffer, size * nitems);
    std::regex pattern("Set-Cookie: ([^=]+)=([^;]+);");
    std::smatch match;
    for (std::sregex_iterator it(header.begin(), header.end(), pattern); it != std::sregex_iterator(); ++it) {
        std::string name = (*it)[1];
        std::string value = (*it)[2];
        cookies[name] = value;
    }
    return size * nitems;
}

std::string cookie_string(const std::map<std::string,std::string>& cookies) {
    std::string cookie_header;
    for (const auto& pair : cookies) { cookie_header += pair.first + "=" + pair.second + "; "; }
    cookie_header = cookie_header.substr(0, cookie_header.length() - 2);
    return cookie_header;
}

void Response::print() {
    std::map<std::string, std::string> cookies = this->cookies;
    printf("Response: %d\n", this->status_code);
    std::cout << "Text: " << this->text << std::endl;
    std::cout << "Json: " << this->json << std::endl;
    std::cout << "Cookies: ";
    std::cout << cookie_string(cookies) + "\n";
}

Response Requests::get(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& proxy) {
    CURL* curl;
    CURLcode res;
    Response response;
    std::string response_text;
    nlohmann::json response_json;
    std::map<std::string, std::string> cookies;
    long status_code = 0;
    struct curl_slist* headerList = NULL;

    //Set headers
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        headerList = curl_slist_append(headerList, header.c_str());
    }

    //Curlopts
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_text);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &cookies);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    if (!proxy.empty()) {
        ProxyInfo p = parse_proxy(proxy);
        curl_easy_setopt(curl, CURLOPT_PROXY, p.proxy.c_str());
        if (!p.user_pass.empty()) {
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, p.user_pass.c_str());
        }
    }


    // Perform the request
    res = curl_easy_perform(curl);
    total_requests++;
    requestCount++;
    if (res != CURLE_OK) {
        total_errors++;
        printf("cURL error: %s\n", curl_easy_strerror(res));
        response = { 0, "failed", response_json, cookies };
    }
    else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        try { response_json = nlohmann::json::parse(response_text); }
        catch (const nlohmann::json::parse_error& e) {}
        response = { (int)status_code, response_text, response_json, cookies };
    }

    curl_slist_free_all(headerList);
    curl_easy_cleanup(curl);
    return response;

}

Response Requests::post(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& data, const std::string& proxy) {
    CURL* curl;
    CURLcode res;
    Response response;
    std::string response_text;
    nlohmann::json response_json;
    std::map<std::string, std::string> cookies;
    long status_code = 0;
    struct curl_slist* headerList = NULL;

    // Set the headers
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        headerList = curl_slist_append(headerList, header.c_str());
    }


    //Curlopts
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &cookies);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_text);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    if (!proxy.empty()) {
        ProxyInfo p = parse_proxy(proxy);
        curl_easy_setopt(curl, CURLOPT_PROXY, p.proxy.c_str());
        if (!p.user_pass.empty()) {
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, p.user_pass.c_str());
        }
    }


    res = curl_easy_perform(curl);
    total_requests++;
    requestCount++;
    if (res != CURLE_OK) {
        total_errors++;
        printf("cURL error: %s\n", curl_easy_strerror(res));
        response = { 0, "failed", response_json, cookies };
    }
    else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        try { response_json = nlohmann::json::parse(response_text); }
        catch (const nlohmann::json::parse_error& e) {}
        response = { (int)status_code, response_text, response_json, cookies };
    }


    curl_slist_free_all(headerList);
    curl_easy_cleanup(curl);
    return response;
}


void discord(const std::string& target, const std::string& url, const std::string& thumbnail, const std::string& avatar, const std::string& content) {
    std::map < std::string, std::string > headers;
    headers["Content-Type"] = "application/json";
    headers["User-Agent"] = "Mozilla/5.0";
    nlohmann::json data = {
        {"content", content},
        {"embeds", {
            {
                {"title", "@" + target + " has been sniped"},
                {"description", "`Username`\n**[" + target + "]('https://instagram.com/" + target + "')**\n`Attempts`\n**" + std::to_string(total_requests) + "**\n\n"},
                {"url", "https://instagram.com/" + target},
                {"color",  get_random_color()},
                {"thumbnail", {
                    {"url", thumbnail}
                }}
            }
        }},
        {"username", "Slimy Sniper"},
        {"avatar_url", avatar}
    };


    std::string json_data = data.dump();
    Requests::post(url, headers, json_data);

}