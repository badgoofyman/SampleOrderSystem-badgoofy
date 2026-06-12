#pragma once
#include <ostream>
#include <string>

namespace ConsoleColor {

constexpr const char* RESET  = "\033[0m";
constexpr const char* WHITE  = "\033[97m";
constexpr const char* CYAN   = "\033[96m";   // 메뉴 번호 [N]
constexpr const char* BLUE   = "\033[94m";   // 테이블 헤더, RESERVED 배지
constexpr const char* YELLOW = "\033[93m";   // 강조 수치, 경고 ※
constexpr const char* GREEN  = "\033[92m";   // 완료 메시지, CONFIRMED 배지
constexpr const char* RED    = "\033[91m";   // 오류, 재고 0 강조
constexpr const char* PURPLE = "\033[95m";   // RELEASE 배지
constexpr const char* ORANGE = "\033[33m";   // PRODUCING 배지

inline void print(std::ostream& out, const std::string& text, const char* color) {
    out << color << text << RESET;
}

inline void println(std::ostream& out, const std::string& text, const char* color) {
    out << color << text << RESET << "\n";
}

} // namespace ConsoleColor
