#include "TransferThreader.h"

void TransferThreader::startThread(std::function<void()> task)
{
    threads.emplace_back(std::move(task));
}

void TransferThreader::joinAll() 
{
    for (auto& t : threads) 
    {
        if (t.joinable()) t.join();
    }
}