//receive_message.cpp
#include "receive_message.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>  

void received_Messages(int clientSocket)
{
    char buffer[1024];
    while (true)
    {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0)
        {
            std::cerr << "Server disconnected \n";
            close(clientSocket);
            exit(1);
        }

        buffer[bytesRead] = '\0';
        std::cout << "Received: " << buffer << std::endl;
    }
}

