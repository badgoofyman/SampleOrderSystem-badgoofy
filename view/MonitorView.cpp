#include "MonitorView.h"
#include "../util/ConsoleColor.h"
#include <iostream>
#include <iomanip>
#include <string>

void MonitorView::printOrderSummary(int reserved, int producing, int confirmed,
                                    int release, std::ostream& out) {
    out << "\n";
    ConsoleColor::println(out, "  ┌────────────────────────────────────────┐", ConsoleColor::BLUE);
    ConsoleColor::println(out, "  │             주문 현황                  │", ConsoleColor::BLUE);
    ConsoleColor::println(out, "  ├────────────────────────────────────────┤", ConsoleColor::BLUE);

    auto printRow = [&](const char* label, int count, const char* color) {
        out << "  │  ";
        ConsoleColor::print(out, label, color);
        out << "  ";
        ConsoleColor::print(out, std::to_string(count), ConsoleColor::YELLOW);
        out << "건\n";
    };

    printRow("RESERVED  (접수 대기)", reserved,  ConsoleColor::BLUE);
    printRow("PRODUCING (생산 중)  ", producing, ConsoleColor::ORANGE);
    printRow("CONFIRMED (출고 대기)", confirmed, ConsoleColor::GREEN);
    printRow("RELEASE   (출고 완료)", release,   ConsoleColor::PURPLE);

    ConsoleColor::println(out, "  └────────────────────────────────────────┘", ConsoleColor::BLUE);
}

void MonitorView::printStockStatus(const std::vector<StockStatus>& statuses,
                                   std::ostream& out) {
    out << "\n";
    ConsoleColor::println(out,
        "  ┌──────────┬──────────────────────────┬────────┬──────────┬────────┐",
        ConsoleColor::BLUE);
    ConsoleColor::print(out,
        "  │ ID       │ 이름                     │  재고  │ 주문대기 │  상태  │",
        ConsoleColor::BLUE);
    out << "\n";
    ConsoleColor::println(out,
        "  ├──────────┼──────────────────────────┼────────┼──────────┼────────┤",
        ConsoleColor::BLUE);

    if (statuses.empty()) {
        ConsoleColor::println(out,
            "  │  등록된 시료가 없습니다.                                       │",
            ConsoleColor::YELLOW);
    }

    for (const auto& ss : statuses) {
        const char* statusColor =
            (ss.status == "고갈") ? ConsoleColor::RED :
            (ss.status == "부족") ? ConsoleColor::YELLOW :
                                    ConsoleColor::GREEN;

        std::string stockStr = std::to_string(ss.stock);
        out << "  │ "
            << std::left << std::setw(9)  << ss.sampleId
            << "│ " << std::setw(25) << ss.sampleName
            << "│ ";
        if (ss.stock == 0)
            ConsoleColor::print(out, stockStr, ConsoleColor::RED);
        else
            out << stockStr;
        int stockPad = 7 - static_cast<int>(stockStr.size());
        if (stockPad > 0) out << std::string(stockPad, ' ');
        out << "│ " << std::setw(9) << ss.pendingQty << "│ ";
        ConsoleColor::print(out, ss.status, statusColor);
        out << "│\n";
    }

    ConsoleColor::println(out,
        "  └──────────┴──────────────────────────┴────────┴──────────┴────────┘",
        ConsoleColor::BLUE);
}
