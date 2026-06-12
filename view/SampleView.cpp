#include "SampleView.h"
#include "../util/ConsoleColor.h"
#include "../util/StringUtil.h"
#include <iostream>
#include <iomanip>
#include <string>

static constexpr int PAGE_SIZE = 5;

void SampleView::printSubMenu(std::ostream& out) {
    out << "\n";
    ConsoleColor::println(out, "  ┌─────────────────────────┐", ConsoleColor::BLUE);
    ConsoleColor::println(out, "  │       시료 관리          │", ConsoleColor::BLUE);
    ConsoleColor::println(out, "  └─────────────────────────┘", ConsoleColor::BLUE);
    ConsoleColor::print(out, "  [1]", ConsoleColor::CYAN);
    out << " 시료 등록\n";
    ConsoleColor::print(out, "  [2]", ConsoleColor::CYAN);
    out << " 시료 목록 조회\n";
    ConsoleColor::print(out, "  [3]", ConsoleColor::CYAN);
    out << " 시료 이름 검색\n";
    ConsoleColor::print(out, "  [0]", ConsoleColor::RED);
    out << " 돌아가기\n";
    out << "  선택: ";
}

int SampleView::getSubMenuChoice(std::istream& in, std::ostream& out) {
    std::string line;
    if (!std::getline(in, line)) return -1;
    try { return std::stoi(line); }
    catch (...) { return -1; }
}

static void printTableHeader(std::ostream& out) {
    ConsoleColor::println(out,
        "  ┌──────────┬──────────────────────────┬──────────────┬────────┬────────┐",
        ConsoleColor::BLUE);
    ConsoleColor::print(out,
        "  │ ID       │ 이름                     │ 생산시간(min)│  수율  │  재고  │",
        ConsoleColor::BLUE);
    out << "\n";
    ConsoleColor::println(out,
        "  ├──────────┼──────────────────────────┼──────────────┼────────┼────────┤",
        ConsoleColor::BLUE);
}

static void printTableRow(const Sample& s, std::ostream& out) {
    bool depleted = (s.stock == 0);
    if (depleted) out << ConsoleColor::RED;
    out << "  │ "
        << std::left << std::setw(9)  << s.id
        << "│ " << std::setw(StringUtil::setWidth(s.name, 25)) << s.name
        << "│ " << std::right << std::setw(12) << s.avgProductionTime
        << "│ " << std::setw(6) << std::fixed << std::setprecision(2) << s.yield << "  "
        << "│ " << std::setw(6) << s.stock << "  │";
    if (depleted) out << ConsoleColor::RESET;
    out << "\n";
}

static void printTableFooter(std::ostream& out) {
    ConsoleColor::println(out,
        "  └──────────┴──────────────────────────┴──────────────┴────────┴────────┘",
        ConsoleColor::BLUE);
}

void SampleView::printList(const std::vector<Sample>& samples, std::ostream& out) {
    if (samples.empty()) {
        ConsoleColor::println(out, "  등록된 시료가 없습니다.", ConsoleColor::YELLOW);
        return;
    }
    printTableHeader(out);
    for (const auto& s : samples)
        printTableRow(s, out);
    printTableFooter(out);
}

void SampleView::printListPaged(const std::vector<Sample>& samples,
                                std::istream& in, std::ostream& out) {
    if (samples.empty()) {
        ConsoleColor::println(out, "  등록된 시료가 없습니다.", ConsoleColor::YELLOW);
        return;
    }
    int total = static_cast<int>(samples.size());
    int pages = (total + PAGE_SIZE - 1) / PAGE_SIZE;
    for (int page = 0; page < pages; ++page) {
        printTableHeader(out);
        int start = page * PAGE_SIZE;
        int end   = std::min(start + PAGE_SIZE, total);
        for (int i = start; i < end; ++i)
            printTableRow(samples[i], out);
        printTableFooter(out);
        out << "  " << (page + 1) << "/" << pages << " 페이지";
        if (page < pages - 1) {
            ConsoleColor::print(out, "  [N]", ConsoleColor::CYAN);
            out << " 다음 페이지, 다른 키는 종료: ";
            std::string line;
            std::getline(in, line);
            if (line != "N" && line != "n") break;
        } else {
            out << "\n";
        }
    }
}

std::string SampleView::inputSampleId(std::istream& in, std::ostream& out) {
    out << "  시료 ID (예: S-001): ";
    std::string line;
    std::getline(in, line);
    return line;
}

std::string SampleView::inputSampleName(std::istream& in, std::ostream& out) {
    out << "  시료명: ";
    std::string line;
    std::getline(in, line);
    return line;
}

int SampleView::inputProductionTime(std::istream& in, std::ostream& out) {
    out << "  평균 생산시간 (min/ea): ";
    std::string line;
    std::getline(in, line);
    try { return std::stoi(line); } catch (...) { return 0; }
}

double SampleView::inputYield(std::istream& in, std::ostream& out) {
    out << "  수율 (0.0 초과 ~ 1.0 이하): ";
    std::string line;
    std::getline(in, line);
    try { return std::stod(line); } catch (...) { return -1.0; }
}

int SampleView::inputStock(std::istream& in, std::ostream& out) {
    out << "  초기 재고: ";
    std::string line;
    std::getline(in, line);
    try { return std::stoi(line); } catch (...) { return 0; }
}

void SampleView::printDuplicateId(std::ostream& out) {
    ConsoleColor::println(out, "  ※ 이미 존재하는 시료 ID입니다. 다시 입력해주세요.", ConsoleColor::YELLOW);
}

void SampleView::printInvalidYield(std::ostream& out) {
    ConsoleColor::println(out, "  ※ 수율은 0.0 초과 1.0 이하여야 합니다. 다시 입력해주세요.", ConsoleColor::YELLOW);
}

std::string SampleView::inputSearchName(std::istream& in, std::ostream& out) {
    out << "  검색할 시료 이름: ";
    std::string line;
    std::getline(in, line);
    return line;
}

void SampleView::printNotFound(std::ostream& out) {
    ConsoleColor::println(out, "  검색 결과가 없습니다.", ConsoleColor::YELLOW);
}

void SampleView::printRegistered(const Sample& s, std::ostream& out) {
    ConsoleColor::print(out, "  ✔ 시료 등록 완료: ", ConsoleColor::GREEN);
    out << "[" << s.id << "] " << s.name << "\n";
}
