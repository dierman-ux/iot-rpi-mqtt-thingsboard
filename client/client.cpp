#include "client.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

Client::Client(const std::string& serverIp, int serverPort) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("[Client] Error setting socket receive timeout");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serverPort);
    servaddr.sin_addr.s_addr = inet_addr(serverIp.c_str());

    len = sizeof(servaddr);
    std::cout << "[Cliente] Conectado a " << serverIp << ":" << serverPort << std::endl;
}

void Client::sendMessage(const std::string& message) {
    std::cout << "[Client] Sending Message: " << message << std::endl;
    sendto(sockfd, message.c_str(), message.size(), MSG_CONFIRM,
           (const struct sockaddr*)&servaddr, len);
}

void Client::sendBinaryMessage(const std::vector<char>& data) {
    std::cout << "[Client] Sending Binary Message: (" << data.size() << " bytes)" << std::endl;
    ssize_t bytesSent = sendto(sockfd, data.data(), data.size(), 0,
                               (const struct sockaddr*)&servaddr, len);
    if (bytesSent < 0) {
        perror("[Client] Error sending binary message");
    } else {
        std::cout << "[Client] Binary message sent successfully!" << std::endl;
    }
}

std::string Client::receiveMessage() {
    char buffer[MAXLINE];
    int n = recvfrom(sockfd, buffer, MAXLINE, 0,
                     (struct sockaddr*)&servaddr, &len);
    if (n < 0) {
        perror("[Client] Error at reception");
        return "";
    }
    buffer[n] = '\0';
    std::string msg(buffer);
    std::cout << "[Client] Message Received: " << msg << std::endl;
    return msg;
}

void Client::closeSocket() {
    std::cout << "[Client] Closing Socket." << std::endl;
    close(sockfd);
}

Client::~Client() {
    closeSocket();
}
