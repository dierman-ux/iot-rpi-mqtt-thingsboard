#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <netinet/in.h>
#include <vector>

#define MAXLINE 1024

class Client {
public:
    Client(const std::string& serverIp, int serverPort);
    void sendMessage(const std::string& message);
    void sendBinaryMessage(const std::vector<char>& data);
    std::string receiveMessage();
    void closeSocket();
    ~Client();

private:
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t len;
};

#endif // CLIENT_H