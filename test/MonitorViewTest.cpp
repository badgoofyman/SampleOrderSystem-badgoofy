#include <gmock/gmock.h>
#include "../view/MonitorView.h"
#include "../model/StockStatus.h"
#include <sstream>

using namespace testing;

static constexpr const char* ANSI_GREEN  = "\033[92m";
static constexpr const char* ANSI_YELLOW = "\033[93m";
static constexpr const char* ANSI_RED    = "\033[91m";

TEST(MonitorViewTest, PrintOrderSummary_ContainsStatusLabels) {
    std::ostringstream out;
    MonitorView::printOrderSummary(2, 1, 3, 4, out);
    EXPECT_THAT(out.str(), HasSubstr("RESERVED"));
    EXPECT_THAT(out.str(), HasSubstr("PRODUCING"));
    EXPECT_THAT(out.str(), HasSubstr("CONFIRMED"));
    EXPECT_THAT(out.str(), HasSubstr("RELEASE"));
}

TEST(MonitorViewTest, PrintOrderSummary_ShowsCounts) {
    std::ostringstream out;
    MonitorView::printOrderSummary(2, 1, 3, 4, out);
    EXPECT_THAT(out.str(), HasSubstr("2"));
    EXPECT_THAT(out.str(), HasSubstr("1"));
    EXPECT_THAT(out.str(), HasSubstr("3"));
    EXPECT_THAT(out.str(), HasSubstr("4"));
}

TEST(MonitorViewTest, PrintStockStatus_ExcessStock_ShowsGreen) {
    std::ostringstream out;
    std::vector<StockStatus> statuses = { { "S-001", "웨이퍼", "여유", 100, 30 } };
    MonitorView::printStockStatus(statuses, out);
    EXPECT_THAT(out.str(), HasSubstr(ANSI_GREEN));
    EXPECT_THAT(out.str(), HasSubstr("여유"));
}

TEST(MonitorViewTest, PrintStockStatus_InsufficientStock_ShowsYellow) {
    std::ostringstream out;
    std::vector<StockStatus> statuses = { { "S-002", "웨이퍼", "부족", 5, 20 } };
    MonitorView::printStockStatus(statuses, out);
    EXPECT_THAT(out.str(), HasSubstr(ANSI_YELLOW));
    EXPECT_THAT(out.str(), HasSubstr("부족"));
}

TEST(MonitorViewTest, PrintStockStatus_DepletedStock_ShowsRed) {
    std::ostringstream out;
    std::vector<StockStatus> statuses = { { "S-003", "웨이퍼", "고갈", 0, 10 } };
    MonitorView::printStockStatus(statuses, out);
    EXPECT_THAT(out.str(), HasSubstr(ANSI_RED));
    EXPECT_THAT(out.str(), HasSubstr("고갈"));
}

TEST(MonitorViewTest, PrintStockStatus_EmptyList_NocrashAndNotEmpty) {
    std::ostringstream out;
    MonitorView::printStockStatus({}, out);
    EXPECT_FALSE(out.str().empty());
}
