#include "Errors.h"

std::string redText(const std::string& text) {
    std::string red = "\033[1;31m";
    std::string reset = "\033[0m";
    return (red + text + reset);
}

std::string yellowText(const std::string& text) {
    std::string yellow = "\033[1;33m";
    std::string reset = "\033[0m";
    return (yellow + text + reset);
}

std::string greenText(const std::string& text) {
    std::string green = "\033[1;32m";
    std::string reset = "\033[0m";
    return (green + text + reset);
}

std::string magentaText(const std::string& text) {
    std::string magenta = "\033[1;35m";
    std::string reset = "\033[0m";
    return (magenta + text + reset);
}

LoginException::LoginException(const std::string& message) : std::runtime_error(message), message_(message) {}

const char* LoginException::what() const noexcept {
    return message_.c_str();
}

EmptyFileException::EmptyFileException(const std::string& message) : std::runtime_error(message), message_(message) {}

const char* EmptyFileException::what() const noexcept {
    return message_.c_str();
}