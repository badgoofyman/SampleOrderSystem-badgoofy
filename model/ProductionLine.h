#pragma once
#include <deque>
#include <string>
#include <ctime>

struct ProductionJob {
    std::string orderNo;
    std::string sampleId;
    int         productionQty;
    long long   totalTimeSeconds;
    time_t      startTime;
};

class ProductionLine {
public:
    void enqueue(const ProductionJob& job);
    bool isEmpty() const;
    const ProductionJob& current() const;
    bool isCurrentComplete() const;
    void completeAndAdvance();
    size_t size() const;
    const std::deque<ProductionJob>& jobs() const;

private:
    std::deque<ProductionJob> jobs_;
};
