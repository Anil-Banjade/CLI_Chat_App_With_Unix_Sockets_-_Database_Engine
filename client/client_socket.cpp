//client_socket.cpp
#include "client_socket.h"

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 

int create_Client_Socket(){
    int clientSocket=socket(AF_INET,SOCK_STREAM,0); 
    if (clientSocket==-1){
        std::cerr<<"Couldn't create client socket\n";
        return -1;
    }

    return clientSocket;
}
