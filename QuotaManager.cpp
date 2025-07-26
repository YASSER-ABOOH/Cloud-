#include "QuotaManager.h"
#include <filesystem>

QuotaManager::QuotaManager() : quotaLimit(100 * 1024 * 1024) 
{}

size_t QuotaManager::getUsedQuota(const std::string& username) const 
{
    std::filesystem::path userPath = "data/" + username;
    size_t totalSize = 0;
    for (const auto& file : std::filesystem::recursive_directory_iterator(userPath)) 
    {
        if (file.is_regular_file()) totalSize += file.file_size();
    }
    return totalSize;
}

bool QuotaManager::canUpload(const std::string& username, size_t fileSize) const 
{
    return (getUsedQuota(username) + fileSize) <= quotaLimit;
}
