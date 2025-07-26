#pragma once
#include <string>

class QuotaManager 
{
private:
    size_t quotaLimit;
public:
    QuotaManager();
    size_t getUsedQuota(const std::string& username) const;
    bool canUpload(const std::string& username, size_t fileSize) const;
};
