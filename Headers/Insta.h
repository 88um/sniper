#pragma once
#include <iostream>
#include "Requests.h"
#include "Utils.h"
#include "Discord.h"

class Insta {
public:
	Response lastResponse();
	Requests::Session requests;
	bool use_proxies = true;
	std::string userId();
	std::string lastText();
	int lastCode();
	bool isClaimed();
	bool isLoggedIn();
	bool profileInfo();
	void removeSelf();
	void handleErrors();
	void claimUser(const std::string& username);
	void setSession(const std::string& session);
	bool login(const std::string& username, const std::string& password);
	bool sendRequest(const std::string& url, const std::string& data = "");
	Insta(std::string session) { this->getData(); this->session = session; this->requests.cookies["sessionid"] = session; this->requests.cookies["ds_user_id"] = this->userId();}
	Insta() {}
	
	

private:
	bool claimed = false;
	bool is_logged_in = false;
	std::string session;
	std::string username;
	std::string password;
	std::string email;
	std::string phone;
	std::string full_name;
	std::string external_url;
	std::string chal_url;
	std::string target;
	std::string csrf;
	std::string bio = "Sniped by joshua :)";
	std::map<std::string, std::string>  headers();
	Response last_response;
	void handleChallenge();
	void getData();
	bool sendCode(const std::string& option);
	bool enterCode(const std::string& code);

};

