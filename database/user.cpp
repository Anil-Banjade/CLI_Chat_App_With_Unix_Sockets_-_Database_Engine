// user.cpp
#include "user.h"
#include <fstream>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include "../utils/utils.h"
#include <algorithm>
#include <cctype>

User::User(const std::string& username, const std::string& password)
    : username(username), password((password)) {}

std::string User::getUsername() const {
    return username;
}

std::string User::getPasswordHash() const {
    return password; 
}



std::string User::hashPassword(const std::string& password) {
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;

    if (!mdctx) {
        // Handle error
        return "";
    }

    if (!EVP_DigestInit_ex(mdctx, md, nullptr)) {
        // Handle error
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    if (!EVP_DigestUpdate(mdctx, password.c_str(), password.length())) {
        // Handle error
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    if (!EVP_DigestFinal_ex(mdctx, hash, &hashLen)) {
        // Handle error
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    EVP_MD_CTX_free(mdctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < hashLen; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

// to make easy
    //std::cout << "Hashed Password: " << ss.str() << std::endl;  // Debug print

    return ss.str();
}



// void User::saveUsersToFile(const std::string& filename, const std::vector<User>& users) {
//     std::ofstream file(filename);

//     if (file.is_open()) {
//         for (const auto& user : users) {
//             file << user.getUsername() << "," << user.getPasswordHash() << "\n";
//         }

//         file.close();
//         std::cout << "Saved users to file: " << filename << std::endl;
//     }else{
//         std::cerr << "Unable to open file for saving users: " << filename << std::endl;
//     }
// }



void User::saveUsersToFile(const std::string& filename, const std::vector<User>& users) {
    std::ofstream file(filename);

    if (file.is_open()) {
        for (const auto& user : users) {
            file << user.getUsername() << "," << user.getPasswordHash() << "\n";
        }

        file.close();
    }
}


void User::loadUsersFromFile(const std::string &filename, std::vector<User> &users) {
    std::ifstream file(filename);

    users.clear();

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find(',');
            if (pos != std::string::npos) {
                std::string username = trimWhitespace(line.substr(0, pos));
                //std::cout<<"check:"<<username<<"\n";
                std::string password = trimWhitespace(line.substr(pos + 1));
                //std::cout<<"check:"<<passwordHash<<"\n";

                // Additional cleanup for passwordHash (remove non-alphanumeric characters)
                // passwordHash.erase(std::remove_if(passwordHash.begin(), passwordHash.end(),
                //                                   [](unsigned char c) { return !std::isalnum(c); }),
                //                    passwordHash.end());

                

                users.emplace_back(username, password);
                //std::cout<<"Test1:"<<username<<" "<<password<<"\n";
                
            }
        }

        file.close();
    }


}








