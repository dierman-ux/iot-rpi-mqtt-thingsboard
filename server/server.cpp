#include "server.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <ifaddrs.h>
#include <netdb.h>
#include <cstdlib>

Server* Server::instance = nullptr;

Server::Server(int port) {
    instance = this;
    std::signal(SIGINT, Server::signalHandler);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        std::exit(EXIT_FAILURE);
    }

    std::memset(&servaddr, 0, sizeof(servaddr));
    std::memset(&cliaddr,  0, sizeof(cliaddr));

    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port        = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        std::exit(EXIT_FAILURE);
    }

    std::cout << "[Server] Listening on port " << port << "\n";
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
    } else {
        char host[NI_MAXHOST];
        for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, nullptr, 0,
                            NI_NUMERICHOST) == 0) {
                std::cout << "  " << ifa->ifa_name << ": " << host << "\n";
            }
        }
        freeifaddrs(ifaddr);
    }

    len = sizeof(cliaddr);
}

Server::~Server() {
    closeSocket();
}

void Server::signalHandler(int signum) {
    std::cout << "\n[Server] Caught signal " << signum
              << ", closing socket and exiting.\n";
    if (instance) instance->closeSocket();
    std::_Exit(EXIT_SUCCESS);
}

std::string Server::receiveMessage() {
    char buffer[2048];
    int n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&cliaddr, &len);
    if (n < 0) {
        perror("recvfrom text");
        return "";
    }
    buffer[n] = '\0';
    return std::string(buffer);
}

std::vector<char> Server::receiveBinaryMessage() {
    char buffer[2048];
    int n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&cliaddr, &len);
    if (n < 0) {
        perror("recvfrom bin");
        return {};
    }
    return std::vector<char>(buffer, buffer + n);
}

void Server::sendReply(const std::string& message) {
    sendto(sockfd, message.c_str(), message.size(), 0,
           (const struct sockaddr *)&cliaddr, len);
}

void Server::closeSocket() {
    if (sockfd >= 0) {
        std::cout << "[Server] Closing socket " << sockfd << "\n";
        close(sockfd);
        sockfd = -1;
    }
}

int Server::getSocketFd() const {
    return sockfd;
}
