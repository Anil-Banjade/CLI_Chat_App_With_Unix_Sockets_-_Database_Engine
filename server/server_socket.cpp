//server_socket.cpp
#include "server_socket.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int create_Server_Socket()
{
    std::cout << "Inside server socket\n";
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Couldn't create socket";
        return -1;
    }

    return serverSocket;
}

