//server.cpp
#include "server_binding.h"
#include "server_socket.h"
#include "channel_chat.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "handle_client.h"
#include <thread>
#include <vector>
#include <mutex>

#include "../database/user.h"

#include "../Database/database.h"

SimpleDatabase database;

int main() 
{
    
    int serverSocket = create_Server_Socket();
    if (serverSocket == -1)
    {
        std::cerr << "Could not create server socket\n";
        return 1;
    }
    if (bind_Server_Socket(serverSocket, 12345))
    {
        std::cout << "Server started successfully\n";
        listen(serverSocket, 5);
        
        database.createTable("channels", {{"channelName", "text"}, {"clientId", "text"}, {"message", "text"}});

        while (true)
        {
            sockaddr_in clientAddress;
            socklen_t clientAddrLen = sizeof(clientAddress); 
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddrLen);

            if (clientSocket == -1)
            {
                std::cerr << "Couldn't accept client connection\n";
                close(serverSocket);
                return 1;
            }

            std::lock_guard<std::mutex> lock(clientSocketsMutex);
            clientSockets.push_back(clientSocket);

            std::thread clientThread(handle_Client, clientSocket);
            clientThread.detach();
        }
    }
    close(serverSocket);

    return 0;
}