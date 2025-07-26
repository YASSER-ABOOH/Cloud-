#include "UploadHandler.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

UploadHandler::UploadHandler() : interrupted(false) {}

bool UploadHandler::saveFile(int clientSocket, const std::string& path) {
    std::ofstream outFile(path, std::ios::binary);
    if (!outFile) return false;

    char buffer[4096];
    int bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        outFile.write(buffer, bytesReceived);
        if (bytesReceived == 0) {
            interrupted = true;
            break;
        }
    }

    return true;
}

bool UploadHandler::receiveFile(int clientSocket, const std::string& username) {
    char filenameBuffer[256];
    int bytes = recv(clientSocket, filenameBuffer, sizeof(filenameBuffer), 0);
    if (bytes <= 0) return false;
    std::string filename(filenameBuffer, bytes);

    std::filesystem::create_directories("data/" + username);
    std::string filePath = "data/" + username + "/" + filename;

    bool success = saveFile(clientSocket, filePath);
    if (!success || wasInterrupted()) {
        send(clientSocket, "Upload interrupted. Please try again.", 40, 0);
        return false;
    }

    send(clientSocket, "Upload successful", 18, 0);
    return true;
}

bool UploadHandler::startUpload(int clientSocket, const std::string& username, const std::string& filename) {
    std::filesystem::create_directories("data/" + username);
    std::string filePath = "data/" + username + "/" + filename;

    bool success = saveFile(clientSocket, filePath);
    if (!success || wasInterrupted()) {
        send(clientSocket, "Upload interrupted. Please try again.", 40, 0);
        return false;
    }

    send(clientSocket, "Upload successful", 18, 0);
    return true;
}

bool UploadHandler::wasInterrupted() const {
    return interrupted;
}
