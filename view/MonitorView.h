#pragma once
#include "../model/StockStatus.h"
#include <iosfwd>
#include <vector>

class MonitorView {
public:
    static void printOrderSummary(int reserved, int producing, int confirmed, int release, std::ostream& out);
    static void printStockStatus(const std::vector<StockStatus>& statuses, std::ostream& out);
};
