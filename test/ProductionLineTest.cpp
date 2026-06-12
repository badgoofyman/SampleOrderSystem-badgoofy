#include <gmock/gmock.h>
#include "../model/ProductionLine.h"
#include <stdexcept>

using namespace testing;

class ProductionLineTest : public Test {
protected:
    ProductionLine line;

    ProductionJob makeJob(const std::string& orderNo, long long totalSeconds, time_t start = 0) {
        return ProductionJob{ orderNo, "S-001", 10, totalSeconds, start };
    }
};

TEST_F(ProductionLineTest, IsEmptyOnConstruction) {
    EXPECT_TRUE(line.isEmpty());
    EXPECT_EQ(line.size(), 0u);
}

TEST_F(ProductionLineTest, EnqueueIncreasesSize) {
    line.enqueue(makeJob("ORD-001", 3600));
    EXPECT_FALSE(line.isEmpty());
    EXPECT_EQ(line.size(), 1u);

    line.enqueue(makeJob("ORD-002", 3600));
    EXPECT_EQ(line.size(), 2u);
}

TEST_F(ProductionLineTest, CurrentThrowsWhenEmpty) {
    EXPECT_THROW(line.current(), std::runtime_error);
}

TEST_F(ProductionLineTest, CurrentReturnsFIFOFirst) {
    line.enqueue(makeJob("ORD-001", 3600));
    line.enqueue(makeJob("ORD-002", 3600));
    EXPECT_EQ(line.current().orderNo, "ORD-001");
}

TEST_F(ProductionLineTest, IsCurrentCompleteReturnsFalseWhenNotDone) {
    time_t now = time(nullptr);
    line.enqueue(makeJob("ORD-001", 9999, now));  // 아직 완료 안 됨
    EXPECT_FALSE(line.isCurrentComplete());
}

TEST_F(ProductionLineTest, IsCurrentCompleteReturnsTrueWhenDone) {
    time_t past = time(nullptr) - 10000;
    line.enqueue(makeJob("ORD-001", 100, past));  // start + 100s < now
    EXPECT_TRUE(line.isCurrentComplete());
}

TEST_F(ProductionLineTest, CompleteAndAdvanceRemovesFirst) {
    line.enqueue(makeJob("ORD-001", 3600));
    line.enqueue(makeJob("ORD-002", 3600));
    line.completeAndAdvance();
    EXPECT_EQ(line.size(), 1u);
    EXPECT_EQ(line.current().orderNo, "ORD-002");
}

TEST_F(ProductionLineTest, CompleteAndAdvanceOnLastMakesEmpty) {
    line.enqueue(makeJob("ORD-001", 3600));
    line.completeAndAdvance();
    EXPECT_TRUE(line.isEmpty());
}

TEST_F(ProductionLineTest, JobsReturnsAllEnqueued) {
    line.enqueue(makeJob("ORD-001", 1000));
    line.enqueue(makeJob("ORD-002", 2000));
    EXPECT_EQ(line.jobs().size(), 2u);
    EXPECT_EQ(line.jobs()[0].orderNo, "ORD-001");
    EXPECT_EQ(line.jobs()[1].orderNo, "ORD-002");
}
