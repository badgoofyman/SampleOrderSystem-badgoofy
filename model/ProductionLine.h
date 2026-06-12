#pragma once
#include <string>
#include <vector>
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
    const std::vector<ProductionJob>& jobs() const;

private:
    std::vector<ProductionJob> jobs_;
};
