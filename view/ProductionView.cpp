#include "ProductionView.h"
#include "../util/ConsoleColor.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>

static std::string formatTime(time_t t) {
    char buf[32] = {};
    struct tm tm_info;
#if defined(_WIN32)
    localtime_s(&tm_info, &t);
#else
    localtime_r(&t, &tm_info);
#endif
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);
    return buf;
}

void ProductionView::printProductionLine(const ProductionLine& line, std::ostream& out) {
    if (line.isEmpty()) {
        printEmpty(out);
        return;
    }

    out << "\n";
    ConsoleColor::println(out,
        "  ┌──────┬──────────────────────┬──────────┬──────┬───────────────────┬──────────────┐",
        ConsoleColor::BLUE);
    ConsoleColor::print(out,
        "  │ 상태 │ 주문번호              │ 시료 ID  │ 생산량│ 완료 예정         │ 잔여시간     │",
        ConsoleColor::BLUE);
    out << "\n";
    ConsoleColor::println(out,
        "  ├──────┼──────────────────────┼──────────┼──────┼───────────────────┼──────────────┤",
        ConsoleColor::BLUE);

    const auto& jobs = line.jobs();
    time_t now = time(nullptr);

    for (size_t i = 0; i < jobs.size(); ++i) {
        const auto& job = jobs[i];
        time_t finishTime = job.startTime + static_cast<time_t>(job.totalTimeSeconds);
        std::string finishStr = formatTime(finishTime);

        out << "  │ ";
        if (i == 0) {
            ConsoleColor::print(out, "생산중", ConsoleColor::ORANGE);
        } else {
            ConsoleColor::print(out, "대기 " + std::to_string(i), ConsoleColor::YELLOW);
        }
        out << " │ "
            << std::left << std::setw(21) << job.orderNo
            << "│ " << std::setw(9) << job.sampleId
            << "│ " << std::right << std::setw(4) << job.productionQty << " │ "
            << finishStr << " │ ";

        // 잔여시간 계산
        long long remaining = static_cast<long long>(finishTime) - static_cast<long long>(now);
        if (remaining <= 0) {
            ConsoleColor::print(out, "완료됨        ", ConsoleColor::GREEN);
        } else {
            long long remMin = remaining / 60;
            ConsoleColor::print(out,
                "완료까지 " + std::to_string(remMin) + "분",
                (i == 0) ? ConsoleColor::ORANGE : ConsoleColor::YELLOW);
            out << " ";
        }
        out << "│\n";
    }

    ConsoleColor::println(out,
        "  └──────┴──────────────────────┴──────────┴──────┴───────────────────┴──────────────┘",
        ConsoleColor::BLUE);
}

void ProductionView::printEmpty(std::ostream& out) {
    ConsoleColor::println(out, "  현재 생산 중인 주문이 없습니다.", ConsoleColor::YELLOW);
}

void ProductionView::printCompleted(const std::string& orderNo, std::ostream& out) {
    out << "  ";
    ConsoleColor::print(out, "✔ 생산 완료: ", ConsoleColor::GREEN);
    out << "[" << orderNo << "] → ";
    ConsoleColor::println(out, "CONFIRMED", ConsoleColor::GREEN);
}
