// client.cpp

#include "client_connection.h"
#include "client_socket.h"
#include "receive_message.h"
#include <unistd.h>
#include <iostream>
#include <thread>
#include <cstring>
#include <sys/socket.h>

int main()
{

    int clientSocket = create_Client_Socket();
    if (clientSocket == -1)
    {
        return 1;
    }

    if (!connect_To_Server(clientSocket, "127.0.0.1", 12345))
    {
        close(clientSocket);
        return 1;
    }

    std::thread receiveThread(received_Messages, clientSocket);
    char message[1024];
    char channelName[100];


std::cout<<"\n";
        std::cout << " **********************************************\n";
            std::cout << " *           Welcome to CLI_Chat!             *\n";
            std::cout << " *    Visit \033[34;4mhttps://github.com/Anil-Banjade\033[0m   *\n"; // Set link text to underlined blue
            std::cout << " *         for more information.              *\n";
            std::cout << " **********************************************\n";
            std::cout<<"\n";
    while (true)
    {

        
        

        std::cout << "Command: ";
        std::cin.getline(message, sizeof(message) - 1); 

        if (strncmp(message, "/join ", 6) == 0)
        {
            strcpy(channelName, &message[6]);
            // send_To_Channel(clientSocket, channelName, message);
            send(clientSocket, message, strlen(message), 0);
        }
        else if (strncmp(message, "/create ", 8) == 0)
        {
            strcpy(channelName, &message[8]);
            // std::cout << "inside client channel creation.\n";
            // send_To_Channel(clientSocket, channelName, message);
            send(clientSocket, message, strlen(message), 0);
            
        }
        else if (strncmp(message, "/channel ", 9) == 0)
        {
            // strcpy(channelName, &message[9]);
            // // Send the message to the specified channel
            // send(clientSocket, message, strlen(message), 0);
            size_t pos = std::string(message + 9).find(' ');
            if (pos != std::string::npos)
            {
                std::string channelName(message + 9, pos);
                std::string channelMessage(message + 9 + pos + 1);

                // Send the message to the specified channel
                send(clientSocket, message, strlen(message), 0);
                std::cout << "Message sent to channel.\n";
            }
        }
        else if (strncmp(message, "@", 1) == 0)
        {
            send(clientSocket, message, strlen(message), 0);
        }
        else if (strncmp(message, "/help", 5) == 0)

        {
            std::cout << "\n";
            std::cout << "\033[1;34m/create channel_name\033[0m\n";
            std::cout << "\033[1;34m➜\033[0m create a chat room.\n"; 
            std::cout << "\033[1;34m/join channel_name\033[0m\n";
            std::cout << "\033[1;34m➜\033[0m join a chat room.\n";
            std::cout << "\033[1;34m/listchannels\033[0m\n";
            std::cout << "\033[1;34m➜\033[0m list all chatrooms.\n";
            std::cout << "\033[1;34m/channel channel_name <message>\033[0m\n";
            std::cout << "\033[1;34m➜\033[0m send a message to a particular chatroom.\n";
            std::cout << "\033[1;34m/register username password\033[0m\n";
            std::cout << "\033[1;34m➜\033[0m registration for users.\n";
            std::cout << "\033[1;34m/login username password\033[0m\n";
            std::cout << "\033[1;34m➜\033[0m login for users.\n";
            std::cout << "\033[1;34m@username <message>\033[0m\n";
            std::cout << "\033[1;34m➜\033[0m send a private msg within the app.\n";
            std::cout << "\033[1;34m/listusers\033[0m\n";
            std::cout << "\033[1;34m➜\033[0m list users in the app.\n";
            std::cout<<"\n";
            
        }

        else
        {

            int bytesSent = send(clientSocket, message, strlen(message), 0);
            if (bytesSent <= 0)
            {
                std::cerr << "Error when sending messages\n";
                close(clientSocket);
                exit(1);
            }
        }
    }

    receiveThread.join();
    close(clientSocket);
    return 0;
}


