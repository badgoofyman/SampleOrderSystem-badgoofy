#include <gmock/gmock.h>
#include "../util/ProductionCalculator.h"

using namespace ProductionCalculator;

// calcProductionQty 정상 케이스
TEST(ProductionCalculatorTest, Qty_20_yield092_returns25) {
    EXPECT_EQ(calcProductionQty(20, 0.92), 25);
}

TEST(ProductionCalculatorTest, Qty_10_yield100_returns12) {
    EXPECT_EQ(calcProductionQty(10, 1.0), 12);
}

TEST(ProductionCalculatorTest, Qty_1_yield050_returns3) {
    EXPECT_EQ(calcProductionQty(1, 0.5), 3);
}

TEST(ProductionCalculatorTest, Qty_9_yield090_returns12) {
    EXPECT_EQ(calcProductionQty(9, 0.9), 12);
}

TEST(ProductionCalculatorTest, Qty_100_yield080_returns139) {
    EXPECT_EQ(calcProductionQty(100, 0.8), 139);
}

// calcProductionQty 예외 케이스
TEST(ProductionCalculatorTest, Qty_shortageZero_throws) {
    EXPECT_THROW(calcProductionQty(0, 0.9), std::invalid_argument);
}

TEST(ProductionCalculatorTest, Qty_shortageNegative_throws) {
    EXPECT_THROW(calcProductionQty(-1, 0.9), std::invalid_argument);
}

TEST(ProductionCalculatorTest, Qty_yieldZero_throws) {
    EXPECT_THROW(calcProductionQty(10, 0.0), std::invalid_argument);
}

TEST(ProductionCalculatorTest, Qty_yieldNegative_throws) {
    EXPECT_THROW(calcProductionQty(10, -0.1), std::invalid_argument);
}

TEST(ProductionCalculatorTest, Qty_yieldOver1_throws) {
    EXPECT_THROW(calcProductionQty(10, 1.01), std::invalid_argument);
}

// calcTotalTimeMinutes 정상 케이스
TEST(ProductionCalculatorTest, Minutes_120avg_25qty_returns3000) {
    EXPECT_EQ(calcTotalTimeMinutes(120, 25), 3000);
}

TEST(ProductionCalculatorTest, Minutes_avgTimeZero_throws) {
    EXPECT_THROW(calcTotalTimeMinutes(0, 25), std::invalid_argument);
}

TEST(ProductionCalculatorTest, Minutes_avgTimeNegative_throws) {
    EXPECT_THROW(calcTotalTimeMinutes(-1, 25), std::invalid_argument);
}

// calcTotalTimeSeconds 정상 케이스
TEST(ProductionCalculatorTest, Seconds_120avg_25qty_returns180000) {
    EXPECT_EQ(calcTotalTimeSeconds(120, 25), 180000LL);
}
