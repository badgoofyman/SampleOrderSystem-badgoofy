#include "MenuView.h"
#include <iostream>
#include <string>

void MenuView::printMainMenu(std::ostream& out) {
    out << "\n========================================\n"
        << "    S-Semi 시료 생산주문관리 시스템\n"
        << "========================================\n"
        << " [1] 시료 관리\n"
        << " [2] 시료 주문\n"
        << " [3] 주문 승인/거절\n"
        << " [4] 모니터링\n"
        << " [5] 생산라인 조회\n"
        << " [6] 출고 처리\n"
        << " [0] 종료\n"
        << "----------------------------------------\n"
        << "선택: ";
}

int MenuView::getMenuChoice(std::istream& in, std::ostream& out) {
    std::string line;
    if (!std::getline(in, line)) return -1;
    try { return std::stoi(line); }
    catch (...) { return -1; }
}

void MenuView::printInvalidChoice(std::ostream& out) {
    out << "잘못된 선택입니다. 다시 입력해주세요.\n";
}
