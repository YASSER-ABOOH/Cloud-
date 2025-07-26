#include "DownloadHandler.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

DownloadHandler::DownloadHandler() : interrupted(false) 
{}

bool DownloadHandler::sendFileContent(int clientSocket, const std::string& path) 
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    char buffer[4096];
    while (!file.eof()) 
    {
        file.read(buffer, sizeof(buffer));
        int bytesSent = send(clientSocket, buffer, file.gcount(), 0);
        if (bytesSent <= 0) 
        {
            interrupted = true;
            return false;
        }
    }

    return true;
}

bool DownloadHandler::sendFile(int clientSocket, const std::string& username, const std::string& filename) 
{
    std::string path = "data/" + username + "/" + filename;

    bool success = sendFileContent(clientSocket, path);
    if (!success || wasInterrupted()) 
    {
        send(clientSocket, "Download interrupted. Please try again.", 41, 0);
        return false;
    }

    send(clientSocket, "Download successful", 20, 0);
    return true;
}

bool DownloadHandler::wasInterrupted() const
{
    return interrupted;
}
