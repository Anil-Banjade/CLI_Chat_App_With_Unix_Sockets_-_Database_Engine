// LoggedInUser.h
#ifndef LOGGEDINUSER_H
#define LOGGEDINUSER_H

#include <string>

class LoggedInUser {
public:
    std::string username;
    int socket;

    LoggedInUser(const std::string &username, int socket);
};

#endif
