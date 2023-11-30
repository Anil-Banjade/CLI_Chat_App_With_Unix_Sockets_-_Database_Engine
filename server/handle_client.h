#ifndef HANDLECLIENT_H
#define HANDLECLIENT_H
#include <mutex> 
#include <vector>
extern std::vector<int> clientSockets;
extern std::mutex clientSocketsMutex;
void handle_Client(int clientSocket);
#endif