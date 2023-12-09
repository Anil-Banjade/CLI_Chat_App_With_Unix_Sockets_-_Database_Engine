// handle_client.cpp
#include "handle_client.h" 
#include "channel_chat.h"
#include "../database/user.h" 
#include <iostream> 
#include <unistd.h> 
#include <sys/socket.h> 
#include <cstring>
#include <vector>
#include <mutex>
#include <algorithm>
#include <map> 

#include "../utils/utils.h"

#include "../Database/database.h"

#include "LoggedInUser.h"

std::vector<LoggedInUser> loggedInUsers; 
std::mutex loggedInUsersMutex;

extern SimpleDatabase database;

std::vector<int> clientSockets;
std::mutex clientSocketsMutex;
std::vector<User> registeredUsers;
std::mutex userMutex;

// mapping to store the channels each client is in.
std::map<int, std::string> clientChannels;


void handle_Client(int clientSocket)
{
    char buffer[1024];

    User::loadUsersFromFile("users.csv", registeredUsers); 

    // for (const auto &user : registeredUsers)
    // {
    //     std::cout << "Username: " << user.username << ", Password Hash: " << user.password << "\n";
    // }

    // bool isLoggedIn = false;

    std::string loggedInUsername;

    while (true)
    {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0); 

        if (bytesRead <= 0)
        {
            std::cerr << "Client disconnected. \n";
            std::lock_guard<std::mutex> lock(clientSocketsMutex);
            clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
            close(clientSocket);
            return; 
        }
        buffer[bytesRead] = '\0';

        if (strncmp(buffer, "/register ", 10) == 0)
        {
            // Register a new user
            std::string userInfo(&buffer[10]);
            size_t pos = userInfo.find(' '); 

            if (pos != std::string::npos)
            {
                std::string username = userInfo.substr(0, pos);
                std::string password = userInfo.substr(pos + 1);

                std::lock_guard<std::mutex> lock(userMutex);
                auto it = std::find_if(registeredUsers.begin(), registeredUsers.end(),
                                       [username](const User &user)
                                       {
                                           return user.getUsername() == username;
                                       });

                if (it == registeredUsers.end())
                {
                    registeredUsers.emplace_back(username, User::hashPassword(password));
                    std::cout << "User '" << username << "' registered successfully.\n";
                
                    // Broadcast the registration success message to all clients
                std::lock_guard<std::mutex> lockCreate(clientSocketsMutex);
                for (const auto &socket : clientSockets)
                {
                    if (socket == clientSocket)
                    {
                        std::string registrationMessage = "\033[34mUser '" + username + "' has registered.\033[0m";
                        //std::string registrationMessage = "User '" + username + "' has registered.";
                        send(socket, registrationMessage.c_str(), registrationMessage.size(), 0);
                    }
                }

                }
                else
                {
                    std::cerr << "User '" << username << "' already exists.\n";
                }
            }
            User::saveUsersToFile("users.csv", registeredUsers);
            std::cout<<"Registration successful \n";
        }

        else if (strncmp(buffer, "/login ", 7) == 0)
        {
            // Login existing user
            std::string userInfo(&buffer[7]);
            size_t pos = userInfo.find(' ');

            if (pos != std::string::npos)
            {
                std::string username = userInfo.substr(0, pos);
                std::string password = userInfo.substr(pos + 1);

                
                username = trimWhitespace(username);
                password = trimWhitespace(password);

                
                // std::cout << "Entered Username: " << username << ", Entered Password: " << password << "\n";

                // Debug print to check the content of registeredUsers
                // std::cout << "Users in registeredUsers:\n";
                // for (const auto &user : registeredUsers)
                // {
                //     std::cout << "Username: " << user.username << ", Password Hash: " << user.password << "\n";
                // }

                std::lock_guard<std::mutex> lock(userMutex);  
                auto it = std::find_if(registeredUsers.begin(), registeredUsers.end(),
                                       [username](const User &user)
                                       {
                                           return user.getUsername() == username;
                                       });

                if (it != registeredUsers.end())
                {
                    std::cout << "Found user with username '" << username << "'. Now checking password hash.\n";

                    // Trim stored and generated passwords
                    std::string storedPassword = trimWhitespace(it->getPasswordHash());
                    // std::string generatedPassword = trimWhitespace(User::hashPassword(password));
                    std::string generatedPassword = trimWhitespace(User::hashPassword(password));

                    std::cout << "Stored Password Hash: " << storedPassword << "\n";
                    std::cout << "Generated Password Hash: " << generatedPassword << "\n";

                    // std::cout << "length of stored password:" << storedPassword.length() << " length of registered user:" << generatedPassword.length() << "\n";

                    
                    if (storedPassword.length() != generatedPassword.length())
                    {
                        std::cerr << "Invalid password.\n";
                    }
                    else if (storedPassword == generatedPassword)
                    {
                        loggedInUsername = username; // for direct message

                        std::lock_guard<std::mutex> lock(loggedInUsersMutex);
                        loggedInUsers.push_back({username, clientSocket}); // Add the user to the list

                        std::cout << "User '" << username << "' logged in.\n";
                        
                        // Broadcast the login success message to all clients
                std::lock_guard<std::mutex> lockCreate(clientSocketsMutex);
                for (const auto &socket : clientSockets)
                {
                    if (socket == clientSocket)
                    {
                        std::string loginMessage = "\033[34mUser '" + username + "' has logged in.\033[0m";
                        //std::string loginMessage = "User '" + username + "' has logged in.";
                        send(socket, loginMessage.c_str(), loginMessage.size(), 0);
                    }
                }
                        
                        // Save users to file after a successful login
                        User::saveUsersToFile("users.csv", registeredUsers);
                        
                    }
                    else
                    {
                        std::cerr << "Invalid password.\n";
                    }
                }
            }
        }

        


    else if (strncmp(buffer, "@", 1) == 0)
{
    std::string message(buffer + 1);
    size_t pos = message.find(' ');

    if (pos != std::string::npos)
    {
        std::string targetUser = message.substr(0, pos);
        std::string content = message.substr(pos + 1);

        std::lock_guard<std::mutex> lock(loggedInUsersMutex);

        // Find the socket of the target user in the list of logged-in users
        auto targetUserIt = std::find_if(loggedInUsers.begin(), loggedInUsers.end(),
                                         [targetUser](const LoggedInUser &user)
                                         {
                                             return user.username == targetUser;
                                         });

        if (targetUserIt != loggedInUsers.end())
        {
            // Send the private message only to the target user
            std::string formattedMessage = "(Private) \033[34m" + loggedInUsername + ": " + content + "\033[0m";

            //std::string formattedMessage = "(Private) " + loggedInUsername + ": " + content;
            send(targetUserIt->socket, formattedMessage.c_str(), formattedMessage.size(), 0);

            // Send an acknowledgment back to the sender
            std::string ackMessage = "Private message sent to " + targetUser;
            send(clientSocket, ackMessage.c_str(), ackMessage.size(), 0);

            // Debugging output
            std::cout << "Sent private message from " << loggedInUsername << " to " << targetUser << ": " << content << std::endl;
        }
        else
        {
            // Send a notification if the target user is not found
            std::string notFoundMessage = "User " + targetUser + " not found";
            send(clientSocket, notFoundMessage.c_str(), notFoundMessage.size(), 0);

            // Debugging output
            std::cout << "User " << targetUser << " not found for private message from " << loggedInUsername << std::endl;
        }
    }
}
        

        else if (strncmp(buffer, "/listusers", 10) == 0)
        {
            std::lock_guard<std::mutex> lock(loggedInUsersMutex);
            std::string userList = "Online Users: ";

            for (const auto &loggedInUser : loggedInUsers)
            {
                if (loggedInUser.socket != clientSocket)
                {
                    userList += loggedInUser.username + ", ";
                }
            }

            if (userList.size() > 14)
            { // Check if there are online users
                userList = userList.substr(0, userList.size() - 2);
                send(clientSocket, userList.c_str(), userList.size(), 0);
            }
            else
            {
                std::string message = "No users online.";
                send(clientSocket, message.c_str(), message.size(), 0);
            }
        }



        else if (strncmp(buffer, "/create ", 8) == 0)
        {
            // Existing channel creation logic
            std::string channelName(&buffer[8]);
            create_Channel(channelName);

            // Broadcast the channel creation message to all clients
            std::lock_guard<std::mutex> lockCreate(clientSocketsMutex);
            for (const auto &socket : clientSockets)
            {
                if (socket == clientSocket)
                {
                    std::string creationMessage = "\033[34mChannel '" + channelName + "' has been created.\033[0m";
                    //std::string creationMessage = "Channel '" + channelName + "' has been created.";
                    send(socket, creationMessage.c_str(), creationMessage.size(), 0);
                }
            }
        }

        // else if (strncmp(buffer, "/join ", 6) == 0)
        // {
        //     std::string channelName(&buffer[6]);

        //     // Check if the channel exists
        //     std::lock_guard<std::mutex> lock(channelMutex);
        //     auto it = channels.find(channelName);



        //     if (it != channels.end())
        //     {
        //         // Update the clientChannels map
        //         std::lock_guard<std::mutex> lock(clientSocketsMutex);
        //         auto clientIt = clientChannels.find(clientSocket);

        //         if (clientIt != clientChannels.end())
        //         {
        //             clientIt->second = channelName; // Update the channel for the client
        //             std::cout << "Joined channel '" << channelName << "'.\n";
        //         }
        //         else
        //         {
        //             // If the client is not in the map, add it with the channel
        //             clientChannels[clientSocket] = channelName;
        //             std::cout << "Joined channel '" << channelName << "'.\n";
        //         }

        //         // Add the client socket to the channel in the channels map
        //         it->second.push_back(clientSocket);

        //         std::lock_guard<std::mutex> lockCreate(clientSocketsMutex);
        //         for (const auto &socket : clientSockets)
        //         {
        //             if (socket == clientSocket)
        //             {
        //                 std::string joinMessage = "\033[34mUser '" + loggedInUsername + "' has joined channel "+ channelName + "'.\033[0m";
        //                 //std::string joinMessage = "User '" + loggedInUsername + "' has joined channel '" + channelName + "'.";
        //                 send(socket, joinMessage.c_str(), joinMessage.size(), 0);
        //             }
        //         }
        //     }
        //     else
        //     {
        //         std::cerr << "Channel '" << channelName << "' does not exist.\n";
        //     }
        // }

        else if (strncmp(buffer, "/join ", 6) == 0)
        {
            std::string channelName(&buffer[6]);

            // Check if the channel exists
            std::lock_guard<std::mutex> lock(channelMutex);
            auto it = channels.find(channelName);

            if (it != channels.end())
            {
                // Update the clientChannels map
                std::lock_guard<std::mutex> lock(clientSocketsMutex);
                auto clientIt = clientChannels.find(clientSocket);

                if (clientIt != clientChannels.end())
                {
                    clientIt->second = channelName; // Update the channel for the client
                    std::cout << "Joined channel '" << channelName << "'.\n";
                }
                else
                {
                    // If the client is not in the map, add it with the channel
                    clientChannels[clientSocket] = channelName;
                    std::cout << "Joined channel '" << channelName << "'.\n";
                }

                // Add the client socket to the channel in the channels map
                it->second.push_back(clientSocket);
            }
            else
            {
                std::cerr << "Channel '" << channelName << "' does not exist.\n";
            }
        }

        else if (strncmp(buffer, "/channel ", 9) == 0)
        {
            size_t pos = std::string(&buffer[9]).find(' ');
            if (pos != std::string::npos)
            {
                std::string channelName(&buffer[9], pos);
                std::string message(&buffer[9 + pos + 1]);

                std::cout << "Channel: " << channelName << ", Message: " << message << "\n"; // Debug print

                // Store the channelName and message in the database
                std::vector<std::string> values = {channelName, std::to_string(clientSocket), message};
                database.insertRow("channels", values);

                // Check if the client is in the specified channel
                std::lock_guard<std::mutex> lock(channelMutex);
                auto it = clientChannels.find(clientSocket);

                if (it != clientChannels.end() && it->second == channelName)
                {
                    std::cout << "You are in channel '" << channelName << "'.\n"; // Debug print

                    // Debug print to check client sockets in the channel
                    std::cout << "Client sockets in channel '" << channelName << "': ";
                    for (const auto &client : channels[channelName])
                    {
                        std::cout << client << ", ";
                    }
                    std::cout << "\n";

                    // Include sender and channel information in the message
                    //std::string formattedMessage = "Channel '" + channelName + "', User '" + std::to_string(clientSocket) + "': " + message;
                    std::string formattedMessage = "\033[34mChannel '" + channelName + "', User '" + std::to_string(clientSocket) + "': " + message + "\033[0m";

                    // Send the message to all clients in the channel, excluding the sender
                    for (const auto &client : channels[channelName])
                    {
                        if (client != clientSocket)
                        {
                            send(client, formattedMessage.c_str(), formattedMessage.size(), 0);
                        }
                    }
                }
                else
                {
                    std::cerr << "You are not in channel '" << channelName << "'.\n";
                }
            }
        }

        else if (strncmp(buffer, "/listchannels", 13) == 0)
        {
            // List available channels
            std::lock_guard<std::mutex> lock(channelMutex);

            if (channels.empty())
            {
                // No channels available
                std::string message = "No channels available.";
                send(clientSocket, message.c_str(), message.size(), 0);
            }
            else
            {
                // Build a string with the list of channels
                std::string channelList = "Available Channels: ";
                for (const auto &channel : channels)
                {
                    channelList += channel.first + ": ";

                    for (const auto &client : channel.second)
                    {
                        channelList += std::to_string(client) + ", ";
                    }

                    channelList += "\n";
                }

                // Remove the trailing comma and space
                channelList = channelList.substr(0, channelList.size() - 2);

                // Send the channel list to the client
                send(clientSocket, channelList.c_str(), channelList.size(), 0);
            }
        }

        else
        {
            // Existing general message sending logic
            std::lock_guard<std::mutex> lock(clientSocketsMutex);
            for (const auto &socket : clientSockets)
            {
                if (socket != clientSocket)
                {
                    send(socket, buffer, strlen(buffer), 0);
                }
            }
        }
    }
}
