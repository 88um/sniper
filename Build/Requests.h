#pragma once
#include <iostream>
#include <string>
#include <map>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <regex>
#include "Utils.h"

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);
size_t header_callback(char* buffer, size_t size, size_t nitems, std::map<std::string, std::string>& cookies);
std::string cookie_string(const std::map<std::string, std::string>& cookies);

struct Response{
    int status_code;
    std::string text;
    nlohmann::json json;
    std::map<std::string, std::string> cookies;
    void print();

    
};


class Requests {
public:
    static Response get(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& proxy = "");
    static Response post(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& data, const std::string& proxy = "");

    class Session {
    public:
        std::map < std::string, std::string> cookies;
        Response get(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& proxy = "");
        Response post(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& data, const std::string& proxy = "");
        Session() {}

    };
};


