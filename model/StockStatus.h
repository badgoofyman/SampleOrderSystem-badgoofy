#pragma once
#include <string>

struct StockStatus {
    std::string sampleId;
    std::string sampleName;
    std::string status;   // "여유" | "부족" | "고갈"
    int         stock;
    int         pendingQty;
};
