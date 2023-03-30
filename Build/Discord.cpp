#include "Discord.h"
#include "Utils.h"

void Discord::send(const std::string& target) {
    std::string webhook_url;
    std::string avatar_url;
    std::string thumbnail_url;
    std::string content;
    nlohmann::json config;
    try {
        std::ifstream file("config.json");
        file >> config;
        webhook_url = config["webhook_url"];
        thumbnail_url = config["thumbnail_url"];
        avatar_url = config["avatar_url"];
        content = config["content"];
        discord(target, webhook_url, thumbnail_url, avatar_url, content);
    }
    catch (std::exception& e) {

    }


}


