#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <array>
#include <iostream>
#include <fstream>

#define BUFFER_SIZE 2048
#define MAX_CLIENT 4
char buffer[BUFFER_SIZE] = {0};
int main(void)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in servAddr = {0};

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(4242);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    while (1)
    {
        std::string bufer;

        std::cout << "Type your message: ";
        getline(std::cin, bufer);

        std::cout << "You sent: " << bufer << std::endl;
        sendto(sockfd, bufer.c_str(), bufer.length(), 0, (struct sockaddr *)&servAddr, sizeof(servAddr));

        recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        std::cout << "You receive: " << bufer << std::endl;
        memset(buffer, 0, BUFFER_SIZE);
    }
}