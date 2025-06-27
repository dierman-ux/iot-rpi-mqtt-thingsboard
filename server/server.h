#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <netinet/in.h>
#include <csignal>

class Server {
public:
    Server(int port = 8080);
    ~Server();

    std::string receiveMessage();
    std::vector<char> receiveBinaryMessage();
    void sendReply(const std::string& message);
    void closeSocket();
    int getSocketFd() const;

    static void signalHandler(int signum);

private:
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    static Server* instance;
};

#endif // SERVER_H