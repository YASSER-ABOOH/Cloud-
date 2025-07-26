#include <iostream>
#include<filesystem>
#include<fstream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

int main() 
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));

    std::string filename = "example.txt";
    send(sock, filename.c_str(), filename.size(), 0);

    std::ifstream file(filename, std::ios::binary);
    char buffer[4096];
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        send(sock, buffer, file.gcount(), 0);
    }

    char response[256];
    int bytes = recv(sock, response, sizeof(response), 0);
    std::cout << std::string(response, bytes) << std::endl;

    closesocket(sock);
    WSACleanup();
    return 0;
}
