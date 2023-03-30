#include "Requests.h"

Response Requests::Session::get(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& proxy) {
    CURL* curl;
    CURLcode res;
    Response response;
    std::string response_text;
    nlohmann::json response_json;
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
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &this->cookies);
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_string(this->cookies).c_str());
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
        response = { 0, "failed", response_json, this->cookies };
    }
    else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        try { response_json = nlohmann::json::parse(response_text); }
        catch (const nlohmann::json::parse_error& e) {}
        response = { (int)status_code, response_text, response_json, this->cookies };
    }

    curl_slist_free_all(headerList);
    curl_easy_cleanup(curl);
    return response;

}

Response Requests::Session::post(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& data, const std::string& proxy) {
    CURL* curl;
    CURLcode res;
    Response response;
    std::string response_text;
    nlohmann::json response_json;
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
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &this->cookies);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_text);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_string(this->cookies).c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
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
        response = { 0, "failed", response_json, this->cookies };
    }
    else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        try { response_json = nlohmann::json::parse(response_text); }
        catch (const nlohmann::json::parse_error& e) {}
        response = { (int)status_code, response_text, response_json, this->cookies };
    }


    curl_slist_free_all(headerList);
    curl_easy_cleanup(curl);
    return response;
}