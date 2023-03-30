#pragma once
#include <map>
#include <thread>
#include <sstream>
#include <vector>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <queue>
#include <fstream>
#include <random>
#include <chrono>
#include <atomic>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <set>
#include <curl/curl.h>
#include <regex>
#include <windows.h>
#include <conio.h>
#include "Errors.h"


extern std::vector<int> color_values;

extern std::atomic<int> requestCount;

extern std::queue<std::string> users;

extern std::queue<std::string> sessions;

extern std::mutex print_lock;

extern std::mutex file_lock;

extern std::string logo;

extern int total_requests;

extern int total_errors;

int get_random_color();

bool load();

void clear_screen();

void track_requests();

void discord(const std::string& target, const std::string& url, const std::string& thumbnail, const std::string& avatar, const std::string& content);

bool file_exists(const std::string& name);

std::string url_encode(const std::string& str);

std::string sign(const std::map<std::string, std::string>& my_map);

std::string random_proxy();

std::string cookie_header(std::map<std::string, std::string>& cookies);

std::queue<std::string> queue_file(std::string filename);

std::string get_user();

std::string get_session();

std::string mask_pass();

struct ProxyInfo {
	std::string username;
	std::string password;
	std::string proxy;
	std::string user_pass;
};


ProxyInfo parse_proxy(const std::string& address);



