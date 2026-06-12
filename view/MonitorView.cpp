#include "MonitorView.h"
#include <iostream>
#include <iomanip>

void MonitorView::printOrderSummary(int reserved, int producing, int confirmed, int release, std::ostream& out) {
    out << "\n--- 주문 현황 ---\n"
        << "  RESERVED  (접수): " << reserved  << "건\n"
        << "  PRODUCING (생산): " << producing << "건\n"
        << "  CONFIRMED (대기): " << confirmed << "건\n"
        << "  RELEASE   (출고): " << release   << "건\n";
}

void MonitorView::printStockStatus(const std::vector<StockStatus>& statuses, std::ostream& out) {
    out << "\n--- 시료별 재고 현황 ---\n"
        << std::left
        << std::setw(8)  << "ID"
        << std::setw(25) << "이름"
        << std::setw(8)  << "재고"
        << std::setw(10) << "주문대기"
        << std::setw(8)  << "상태\n"
        << std::string(59, '-') << "\n";
    for (const auto& ss : statuses) {
        out << std::left
            << std::setw(8)  << ss.sampleId
            << std::setw(25) << ss.sampleName
            << std::setw(8)  << ss.stock
            << std::setw(10) << ss.pendingQty
            << std::setw(8)  << ss.status << "\n";
    }
}
