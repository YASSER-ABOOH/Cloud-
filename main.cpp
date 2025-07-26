#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include "UploadHandler.h"
#include "DownloadHandler.h"
#include "UploadLogger.h"
#include "FileLister.h"

#pragma comment(lib, "ws2_32.lib")

void sendVectorOverSocket(const std::vector<std::string>& data, int clientSocket) {
    for (const std::string& item : data) {
        send(clientSocket, item.c_str(), static_cast<int>(item.size()), 0);
        send(clientSocket, "\n", 1, 0);  // Line break after each item
    }
    send(clientSocket, "END_OF_LIST", 12, 0);  // Termination signal
}

void startServer() {
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr{}, clientAddr{};
    int clientLen = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    listen(serverSocket, 5);
    std::cout << "Server listening on port 8080...\n";

    while (true) {
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed.\n";
            continue;
        }

        char opBuffer[4] = {};
        if (recv(clientSocket, opBuffer, sizeof(opBuffer), 0) <= 0) {
            std::cerr << "Operation code recv failed.\n";
            closesocket(clientSocket);
            continue;
        }
        int operation = std::stoi(opBuffer);

        char userBuffer[256] = {};
        int userBytes = recv(clientSocket, userBuffer, sizeof(userBuffer), 0);
        if (userBytes <= 0) {
            std::cerr << "Username recv failed.\n";
            closesocket(clientSocket);
            continue;
        }
        std::string username(userBuffer, userBytes);

        switch (operation) {
        case 1: {
            std::cout << "[UPLOAD] Receiving file for user: " << username << "\n";
            UploadHandler handler;
            if (!handler.receiveFile(clientSocket, username)) {
                std::string msg = "Upload interrupted. Please try again.\n";
                send(clientSocket, msg.c_str(), msg.size(), 0);
            }
            break;
        }
        case 2: {
            std::cout << "[DOWNLOAD] Receiving filename from client for user: " << username << "\n";
            char fileBuffer[256] = {};
            int fileBytes = recv(clientSocket, fileBuffer, sizeof(fileBuffer), 0);
            if (fileBytes <= 0) {
                std::cerr << "Filename recv failed.\n";
                break;
            }
            std::string filename(fileBuffer, fileBytes);

            DownloadHandler downloader;
            if (!downloader.sendFile(clientSocket, username, filename)) {
                std::string msg = "Download interrupted. Please try again.\n";
                send(clientSocket, msg.c_str(), msg.size(), 0);
            }
            break;
        }
        case 3: {
            std::cout << "[LOGGER] Logging upload...\n";
            char fileBuffer[256] = {};
            int fileBytes = recv(clientSocket, fileBuffer, sizeof(fileBuffer), 0);
            if (fileBytes <= 0) {
                std::cerr << "Filename recv failed for logger.\n";
                break;
            }
            std::string filename(fileBuffer, fileBytes);
            UploadLogger::logUpload(username, filename);
            std::cout << "Logged upload for user: " << username << ", file: " << filename << "\n";
            break;
        }
        case 4: {
            std::cout << "[LIST FILES] Sending file list to client for user: " << username << "\n";
            FileLister lister;
            std::vector<std::string> files = lister.listUserFiles(username);
            sendVectorOverSocket(files, clientSocket);
            break;
        }
        default: {
            std::string msg = "Invalid operation.\n";
            send(clientSocket, msg.c_str(), msg.size(), 0);
            std::cerr << "Unknown operation code: " << operation << "\n";
            break;
        }
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    startServer();
    return 0;
}
