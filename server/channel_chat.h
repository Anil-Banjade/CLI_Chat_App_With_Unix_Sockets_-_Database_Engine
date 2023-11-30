#ifndef CHANNEL_CHAT_H
#define CHANNEL_CHAT_H
#include <string>
#include <mutex>
#include <vector>
#include <map>


extern std::map<std::string,std::vector<int>> channels;

extern std::mutex channelMutex;
void create_Channel(const std::string& channelName);
bool join_Channel(const std::string& channelName, int clientSocket);
void send_To_Channel(const std::string& channelName, const char* message, int senderSocket);
#endif