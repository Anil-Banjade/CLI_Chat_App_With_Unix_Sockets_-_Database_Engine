//server_binding.cpp
#include "server_binding.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 

bool bind_Server_Socket(int serverSocket, int port)
{
    std::cout << "Inside server binding\n";
    sockaddr_in serverAddress;                  
    serverAddress.sin_family = AF_INET;         
    serverAddress.sin_port = htons(port);       
    serverAddress.sin_addr.s_addr = INADDR_ANY; 

    
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cerr << "Couldn't bind to port\n";
        close(serverSocket);

        return false;
    }

    return true;
}

