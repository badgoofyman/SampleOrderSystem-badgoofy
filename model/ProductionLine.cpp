#include "ProductionLine.h"
#include <stdexcept>

void ProductionLine::enqueue(const ProductionJob& job) {
    jobs_.push_back(job);
}

bool ProductionLine::isEmpty() const {
    return jobs_.empty();
}

const ProductionJob& ProductionLine::current() const {
    if (jobs_.empty())
        throw std::runtime_error("ProductionLine is empty");
    return jobs_.front();
}

bool ProductionLine::isCurrentComplete() const {
    const auto& job = current();
    return time(nullptr) >= job.startTime + static_cast<time_t>(job.totalTimeSeconds);
}

void ProductionLine::completeAndAdvance() {
    if (!jobs_.empty())
        jobs_.erase(jobs_.begin());
}

size_t ProductionLine::size() const {
    return jobs_.size();
}

const std::vector<ProductionJob>& ProductionLine::jobs() const {
    return jobs_;
}
