//channel_chat.cpp
#include "channel_chat.h"
#include <iostream>
#include <map>
#include <vector>
#include <mutex>
#include <cstring>
#include <sys/socket.h> 

std::map<std::string,std::vector<int>> channels;



std::mutex channelMutex;

void create_Channel(const std::string& channelName) { 
    std::lock_guard<std::mutex> lock(channelMutex);
    channels[channelName] = std::vector<int>();
    std::cout << "Channel '" << channelName << "' created successfully.\n";
}

bool join_Channel(const std::string& channelName, int clientSocket) {
    std::lock_guard<std::mutex> lock(channelMutex);
    if (channels.find(channelName) != channels.end()) {
        channels[channelName].push_back(clientSocket);
        std::cout << "Joined channel '" << channelName << "'.\n";
        return true;
    }
    return false; 
}

void send_To_Channel(const std::string& channelName, const char* message, int senderSocket) {
    std::lock_guard<std::mutex> lock(channelMutex);
    if (channels.find(channelName) != channels.end()) {
        for (int socket : channels[channelName]) {
            if (socket != senderSocket) {
                send(socket, message, strlen(message), 0);
            }
        }
    }
}
