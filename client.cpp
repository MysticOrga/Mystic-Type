#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <array>
#include <iostream>
#include <fstream>
#include <sys/select.h>

#define BUFFER_SIZE 2048
#define MAX_CLIENT 4
char buffer[BUFFER_SIZE] = {0};
typedef struct packet_s {
    int magic;
    int len;
    char titi[49];
} packet_t;

int main(void)
{
    int nfds = 2;
    fd_set readfds = {0};
    fd_set writefds = {0};
    fd_set exceptfds = {0};
    packet_t pack;

    pack.magic = 120;
    pack.len = 49;
    memset(&pack.titi, 50, 49);
    struct timeval timeout = {
        .tv_sec = 2,
        .tv_usec = 0};

    std::cout << "Creating socket..." << std::endl;
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in addr = {0};

    if (sockfd < 0)
    {
        perror("socket creation failed");
        return EXIT_FAILURE;
    }

    std::cout << "Binding socket..." << std::endl;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4242);
    addr.sin_addr.s_addr = INADDR_ANY;

    // if (bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0)
    // {
    //     perror("bind failed");
    //     close(sockfd);
    //     return EXIT_FAILURE;
    // }
    sendto(sockfd, &pack, sizeof(packet_t), 0, (struct sockaddr*)&addr, sizeof(addr));
    // write(sockfd, &pack, sizeof(packet_t));
//     std::cout << "Entering main loop..." << std::endl;
//     while (1)
//     {
//         FD_ZERO(&readfds);
//         FD_ZERO(&writefds);
//         FD_ZERO(&exceptfds);

//         FD_SET(sockfd, &readfds); // Monitor standard input (fd 0) for reading
//         FD_SET(sockfd, &writefds); // Monitor standard input (fd 0) for reading
//         FD_SET(sockfd, &exceptfds); // Monitor standard input (fd 0) for reading

//         timeout.tv_sec = 2;
//         timeout.tv_usec = 50;
//         int activity = select(sockfd + 1, &readfds, NULL, &exceptfds, &timeout);
//         if (activity < 0)
//         {
//             perror("select error");
//             return EXIT_FAILURE;
//         }

//         if (activity)
//         {
//             std::cout << "Event detected!" << std::endl;
//             if (FD_ISSET(sockfd, &readfds))
//             {
//                 ssize_t bytesRead = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
//                 std::cout << "Data received: " << buffer << std::endl;
//             }
//             else
//             {
//                 std::cout << "No data available to read." << std::endl;
//             }
//             // if (FD_ISSET(sockfd, &writefds))
//             // {
//             //     std::cout << "Socket is ready for writing." << std::endl;
//             // }
//             // else
//             // {
//             //     std::cout << "Socket is not ready for writing." << std::endl;
//             // }
//             if (FD_ISSET(sockfd, &exceptfds))
//             {
//                 std::cout << "Exception on socket." << std::endl;
//             }
//             else
//             {
//                 std::cout << "No exception on socket." << std::endl;
//             }
//         } else
//         {
//             std::cout << "No events within timeout period." << std::endl;
//         }
//     }
    return EXIT_SUCCESS;
}
// int main(void)
// {
//     int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//     sockaddr_in servAddr = {0};

//     servAddr.sin_family = AF_INET;
//     servAddr.sin_port = htons(4242);
//     servAddr.sin_addr.s_addr = INADDR_ANY;

//     while (1)
//     {
//         std::string bufer;

//         std::cout << "Type your message: ";
//         getline(std::cin, bufer);

//         std::cout << "You sent: " << bufer << std::endl;
//         sendto(sockfd, bufer.c_str(), bufer.length(), 0, (struct sockaddr *)&servAddr, sizeof(servAddr));

//         recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
//         std::cout << "You receive: " << bufer << std::endl;
//         memset(buffer, 0, BUFFER_SIZE);
//     }
// }