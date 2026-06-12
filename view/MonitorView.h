#pragma once
#include <iosfwd>
#include <string>
#include <vector>

struct StockStatus {
    std::string sampleId;
    std::string sampleName;
    std::string status;   // "여유" | "부족" | "고갈"
    int         stock;
    int         pendingQty;
};

class MonitorView {
public:
    static void printOrderSummary(int reserved, int producing, int confirmed, int release, std::ostream& out);
    static void printStockStatus(const std::vector<StockStatus>& statuses, std::ostream& out);
};
