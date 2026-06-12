#include "SampleView.h"
#include <iostream>
#include <iomanip>
#include <string>

void SampleView::printSubMenu(std::ostream& out) {
    out << "\n--- 시료 관리 ---\n"
        << " [1] 시료 등록\n"
        << " [2] 시료 목록 조회\n"
        << " [3] 시료 이름 검색\n"
        << " [0] 돌아가기\n"
        << "선택: ";
}

int SampleView::getSubMenuChoice(std::istream& in, std::ostream& out) {
    std::string line;
    if (!std::getline(in, line)) return -1;
    try { return std::stoi(line); }
    catch (...) { return -1; }
}

void SampleView::printList(const std::vector<Sample>& samples, std::ostream& out) {
    if (samples.empty()) {
        out << "등록된 시료가 없습니다.\n";
        return;
    }
    out << "\n"
        << std::left
        << std::setw(8)  << "ID"
        << std::setw(25) << "이름"
        << std::setw(15) << "생산시간(min)"
        << std::setw(10) << "수율"
        << std::setw(8)  << "재고\n"
        << std::string(66, '-') << "\n";
    for (const auto& s : samples) {
        out << std::left
            << std::setw(8)  << s.id
            << std::setw(25) << s.name
            << std::setw(15) << s.avgProductionTime
            << std::setw(10) << std::fixed << std::setprecision(2) << s.yield
            << std::setw(8)  << s.stock << "\n";
    }
}

Sample SampleView::inputNewSample(std::istream& in, std::ostream& out) {
    Sample s;
    out << "\n--- 시료 등록 ---\n";

    out << "시료 ID (예: S-001): ";
    std::getline(in, s.id);

    out << "시료명: ";
    std::getline(in, s.name);

    out << "평균 생산시간 (min/ea): ";
    std::string line;
    std::getline(in, line);
    try { s.avgProductionTime = std::stoi(line); } catch (...) { s.avgProductionTime = 0; }

    out << "수율 (0.0 ~ 1.0): ";
    std::getline(in, line);
    try { s.yield = std::stod(line); } catch (...) { s.yield = 0.0; }

    out << "초기 재고: ";
    std::getline(in, line);
    try { s.stock = std::stoi(line); } catch (...) { s.stock = 0; }

    return s;
}

std::string SampleView::inputSearchName(std::istream& in, std::ostream& out) {
    out << "검색할 시료 이름: ";
    std::string line;
    std::getline(in, line);
    return line;
}

void SampleView::printNotFound(std::ostream& out) {
    out << "검색 결과가 없습니다.\n";
}

void SampleView::printRegistered(const Sample& s, std::ostream& out) {
    out << "시료가 등록되었습니다: [" << s.id << "] " << s.name << "\n";
}
