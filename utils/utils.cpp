// utils.cpp
#include "utils.h"
#include <iostream>

std::string trimWhitespace(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    
    if (start == std::string::npos || end == std::string::npos)
        return "";
    
    return str.substr(start, end - start + 1);
}
