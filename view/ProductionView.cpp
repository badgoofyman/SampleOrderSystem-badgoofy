#include "ProductionView.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>

void ProductionView::printProductionLine(const ProductionLine& line, std::ostream& out) {
    if (line.isEmpty()) {
        printEmpty(out);
        return;
    }
    const auto& jobs = line.jobs();
    out << "\n--- 생산라인 현황 ---\n";
    for (size_t i = 0; i < jobs.size(); ++i) {
        const auto& job = jobs[i];
        time_t finishTime = job.startTime + static_cast<time_t>(job.totalTimeSeconds);
        char buf[32] = {};
        struct tm tm_info;
#if defined(_WIN32)
        localtime_s(&tm_info, &finishTime);
#else
        localtime_r(&finishTime, &tm_info);
#endif
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);

        if (i == 0)
            out << "  [생산중] ";
        else
            out << "  [대기 " << i << "] ";
        out << job.orderNo << " / " << job.sampleId
            << " / 생산량:" << job.productionQty
            << " / 완료예정:" << buf << "\n";
    }
}

void ProductionView::printEmpty(std::ostream& out) {
    out << "현재 생산 중인 주문이 없습니다.\n";
}

void ProductionView::printCompleted(const std::string& orderNo, std::ostream& out) {
    out << "생산 완료 처리: [" << orderNo << "] → CONFIRMED\n";
}
