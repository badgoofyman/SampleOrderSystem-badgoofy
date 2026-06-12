#include "MenuView.h"
#include "../util/ConsoleColor.h"
#include <iostream>
#include <string>

void MenuView::printMainMenu(const SystemStatus& status, std::ostream& out) {
    out << "\n";
    ConsoleColor::println(out, "╔══════════════════════════════════════════╗", ConsoleColor::CYAN);
    ConsoleColor::println(out, "║         S-Semi 시료 생산주문관리            ║", ConsoleColor::CYAN);
    ConsoleColor::println(out, "╚══════════════════════════════════════════╝", ConsoleColor::CYAN);

    out << "  시료 ";
    ConsoleColor::print(out, std::to_string(status.sampleCount), ConsoleColor::YELLOW);
    out << "종  |  총 재고 ";
    ConsoleColor::print(out, std::to_string(status.totalStock), ConsoleColor::YELLOW);
    out << "개  |  주문 ";
    ConsoleColor::print(out, std::to_string(status.orderCount), ConsoleColor::YELLOW);
    out << "건  |  생산 중 ";
    ConsoleColor::print(out, std::to_string(status.producingCount), ConsoleColor::ORANGE);
    out << "건\n";

    out << "  ──────────────────────────────────────────\n";
    ConsoleColor::print(out, "  [1]", ConsoleColor::CYAN);
    out << " 시료 관리          ";
    ConsoleColor::print(out, "[2]", ConsoleColor::CYAN);
    out << " 시료 주문\n";
    ConsoleColor::print(out, "  [3]", ConsoleColor::CYAN);
    out << " 주문 승인/거절      ";
    ConsoleColor::print(out, "[4]", ConsoleColor::CYAN);
    out << " 모니터링\n";
    ConsoleColor::print(out, "  [5]", ConsoleColor::CYAN);
    out << " 생산라인 조회       ";
    ConsoleColor::print(out, "[6]", ConsoleColor::CYAN);
    out << " 출고 처리\n";
    ConsoleColor::print(out, "  [0]", ConsoleColor::RED);
    out << " 종료\n";
    out << "  ──────────────────────────────────────────\n";
    out << "  선택: ";
}

int MenuView::getMenuChoice(std::istream& in, std::ostream& out) {
    std::string line;
    if (!std::getline(in, line)) return -1;
    try { return std::stoi(line); }
    catch (...) { return -1; }
}

void MenuView::printInvalidChoice(std::ostream& out) {
    ConsoleColor::println(out, "  잘못된 선택입니다. 다시 입력해주세요.", ConsoleColor::RED);
}
