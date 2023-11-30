//user.h
#pragma once

#include <string>
#include <vector>
#include <mutex>

class User {
public:
    User(const std::string& username, const std::string& password);

    std::string getUsername() const;
    std::string getPasswordHash() const;
    static std::string hashPassword(const std::string& password);

    static void loadUsersFromFile(const std::string& filename, std::vector<User>& users);
    static void saveUsersToFile(const std::string& filename, const std::vector<User>& users);


    std::string username;
    std::string password;
};

extern std::vector<User> registeredUsers;