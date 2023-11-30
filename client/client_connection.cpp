//client_connection.cpp
#include "client_connection.h" 
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 

bool connect_To_Server(int clientSocket, const char *ipAddress, int port)
{
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;          
    serverAddress.sin_port = htons(port);   
    serverAddress.sin_addr.s_addr = inet_addr(ipAddress);
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cerr << "Couldn't connect to server";
        close(clientSocket);
        return false;
    }

    return true;
}


