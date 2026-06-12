#include <gmock/gmock.h>
#include "../view/ProductionView.h"
#include "../model/ProductionLine.h"
#include <sstream>
#include <ctime>

using namespace testing;

static constexpr const char* ANSI_GREEN  = "\033[92m";
static constexpr const char* ANSI_YELLOW = "\033[93m";
static constexpr const char* ANSI_ORANGE = "\033[33m";

TEST(ProductionViewTest, PrintEmpty_ShowsMessage) {
    std::ostringstream out;
    ProductionLine line;
    ProductionView::printProductionLine(line, out);
    EXPECT_THAT(out.str(), HasSubstr("없습니다"));
}

TEST(ProductionViewTest, PrintCompleted_ShowsOrderNoAndGreen) {
    std::ostringstream out;
    ProductionView::printCompleted("ORD-001", out);
    EXPECT_THAT(out.str(), HasSubstr("ORD-001"));
    EXPECT_THAT(out.str(), HasSubstr("CONFIRMED"));
    EXPECT_THAT(out.str(), HasSubstr(ANSI_GREEN));
}

TEST(ProductionViewTest, PrintProductionLine_ActiveJob_ShowsOrange) {
    std::ostringstream out;
    ProductionLine line;
    time_t future = time(nullptr) + 9999;
    line.enqueue({ "ORD-001", "S-001", 10, 9999, future });
    ProductionView::printProductionLine(line, out);
    EXPECT_THAT(out.str(), HasSubstr("ORD-001"));
    EXPECT_THAT(out.str(), HasSubstr(ANSI_ORANGE));
}

TEST(ProductionViewTest, PrintProductionLine_ActiveJob_ShowsRemainingTime) {
    std::ostringstream out;
    ProductionLine line;
    time_t future = time(nullptr) + 600;  // 10분 후 완료
    line.enqueue({ "ORD-001", "S-001", 5, 600, future });
    ProductionView::printProductionLine(line, out);
    EXPECT_THAT(out.str(), HasSubstr("완료까지"));
}

TEST(ProductionViewTest, PrintProductionLine_MultipleJobs_ShowsFifoOrder) {
    std::ostringstream out;
    ProductionLine line;
    time_t now = time(nullptr);
    time_t future1 = now + 300;
    time_t future2 = now + 600;
    line.enqueue({ "ORD-001", "S-001", 5, 300, future1 });
    line.enqueue({ "ORD-002", "S-002", 3, 600, future2 });
    ProductionView::printProductionLine(line, out);
    // ORD-001이 생산중, ORD-002가 대기
    std::string s = out.str();
    size_t pos1 = s.find("ORD-001");
    size_t pos2 = s.find("ORD-002");
    EXPECT_NE(pos1, std::string::npos);
    EXPECT_NE(pos2, std::string::npos);
    EXPECT_LT(pos1, pos2);  // ORD-001이 먼저 출력
}

TEST(ProductionViewTest, PrintProductionLine_WaitingJob_ShowsYellow) {
    std::ostringstream out;
    ProductionLine line;
    time_t now = time(nullptr);
    line.enqueue({ "ORD-001", "S-001", 5, 300, now + 300 });
    line.enqueue({ "ORD-002", "S-002", 3, 300, now + 600 });
    ProductionView::printProductionLine(line, out);
    EXPECT_THAT(out.str(), HasSubstr(ANSI_YELLOW));
    EXPECT_THAT(out.str(), HasSubstr("대기"));
}
