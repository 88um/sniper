#pragma once

#include <stdexcept>
#include <string>

class LoginException : public std::runtime_error {
public:
    explicit LoginException(const std::string& message);
    const char* what() const noexcept;
private:
    std::string message_;
};



class EmptyFileException : public std::runtime_error {
public:
    explicit EmptyFileException(const std::string& message);
    const char* what() const noexcept;

private:
    std::string message_;
};



std::string redText(const std::string& text);
std::string greenText(const std::string& text);
std::string yellowText(const std::string& text);
std::string magentaText(const std::string& text);

