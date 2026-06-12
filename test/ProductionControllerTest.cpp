#include <gmock/gmock.h>
#include "MockRepositories.h"
#include "../controller/ProductionController.h"
#include "../model/ProductionLine.h"
#include <sstream>

using namespace testing;

class ProductionControllerTest : public Test {
protected:
    MockSampleRepository sampleRepo;
    MockOrderRepository  orderRepo;
    ProductionLine       line;

    std::istringstream in;
    std::ostringstream out;

    std::unique_ptr<ProductionController> ctrl;

    void setup() {
        ctrl = std::make_unique<ProductionController>(sampleRepo, orderRepo, line, out);
    }

    Sample makeSample(const std::string& id, int stock) {
        Sample s;
        s.id = id; s.name = "시료"; s.avgProductionTime = 60; s.yield = 0.9; s.stock = stock;
        return s;
    }
};

TEST_F(ProductionControllerTest, ShowProductionLine_EmptyLine_PrintsEmpty) {
    setup();
    ctrl->showProductionLine();
    EXPECT_THAT(out.str(), HasSubstr("없습니다"));
}

TEST_F(ProductionControllerTest, ShowProductionLine_CompletedJob_AdvancesAndUpdates) {
    // 이미 완료된 잡을 생산라인에 넣고 showProductionLine 호출
    time_t past = time(nullptr) - 9999;
    line.enqueue({ "ORD-001", "S-001", 10, 100, past });  // 완료 상태

    Order o;
    o.orderNo = "ORD-001"; o.sampleId = "S-001"; o.quantity = 5;
    // PRODUCING 상태로 만들기
    o.approve(false);
    o.productionQty = 10;

    EXPECT_CALL(orderRepo, findById("ORD-001")).WillOnce(Return(o));
    EXPECT_CALL(sampleRepo, findById("S-001")).WillOnce(Return(makeSample("S-001", 0)));
    EXPECT_CALL(orderRepo, update(_)).Times(1);
    EXPECT_CALL(sampleRepo, update(_)).Times(1);

    setup();
    ctrl->showProductionLine();

    EXPECT_THAT(out.str(), HasSubstr("CONFIRMED"));
    EXPECT_TRUE(line.isEmpty());
}

TEST_F(ProductionControllerTest, ShowProductionLine_InProgressJob_NoUpdate) {
    time_t future = time(nullptr) + 9999;
    line.enqueue({ "ORD-002", "S-001", 10, 99999, future });

    setup();
    ctrl->showProductionLine();

    EXPECT_FALSE(line.isEmpty());
    EXPECT_THAT(out.str(), HasSubstr("ORD-002"));
}

TEST_F(ProductionControllerTest, ShowProductionLine_MultipleJobs_FifoOrderPreserved) {
    time_t now = time(nullptr);
    // 첫 번째 잡은 미완료, 두 번째도 미완료
    line.enqueue({ "ORD-001", "S-001", 5, 99999, now + 99999 });
    line.enqueue({ "ORD-002", "S-002", 3, 99999, now + 199999 });

    setup();
    ctrl->showProductionLine();

    // 두 잡 모두 출력되어야 하고 ORD-001이 current(생산중)
    EXPECT_EQ(line.size(), 2u);
    EXPECT_EQ(line.current().orderNo, "ORD-001");
    EXPECT_THAT(out.str(), HasSubstr("ORD-001"));
    EXPECT_THAT(out.str(), HasSubstr("ORD-002"));
}

TEST_F(ProductionControllerTest, ShowProductionLine_CompletedJob_StockUpdatedCorrectly) {
    // productionQty=12, quantity=10 → stock 증가분 = 12 - 10 = 2
    time_t past = time(nullptr) - 9999;
    line.enqueue({ "ORD-001", "S-001", 12, 100, past });

    Order o;
    o.orderNo = "ORD-001"; o.sampleId = "S-001"; o.quantity = 10;
    o.approve(false);
    o.productionQty = 12;

    Sample s = makeSample("S-001", 0);

    EXPECT_CALL(orderRepo, findById("ORD-001")).WillOnce(Return(o));
    EXPECT_CALL(sampleRepo, findById("S-001")).WillOnce(Return(s));

    // stock += productionQty - quantity = 12 - 10 = 2
    EXPECT_CALL(sampleRepo, update(Field(&Sample::stock, 2))).Times(1);
    EXPECT_CALL(orderRepo, update(_)).Times(1);

    setup();
    ctrl->showProductionLine();

    EXPECT_TRUE(line.isEmpty());
}

// ===== processCompletedJobs (public) =====

TEST_F(ProductionControllerTest, ProcessCompletedJobs_CompletedJob_SetsConfirmedAndUpdatesStock) {
    time_t past = time(nullptr) - 9999;
    line.enqueue({ "ORD-001", "S-001", 10, 100, past });

    Order o;
    o.orderNo = "ORD-001"; o.sampleId = "S-001"; o.quantity = 5;
    o.approve(false);
    o.productionQty = 10;

    EXPECT_CALL(orderRepo, findById("ORD-001")).WillOnce(Return(o));
    EXPECT_CALL(sampleRepo, findById("S-001")).WillOnce(Return(makeSample("S-001", 0)));
    EXPECT_CALL(sampleRepo, update(Field(&Sample::stock, 5))).Times(1);
    EXPECT_CALL(orderRepo, update(_)).Times(1);

    setup();
    ctrl->processCompletedJobs();  // public 직접 호출

    EXPECT_TRUE(line.isEmpty());
    EXPECT_THAT(out.str(), HasSubstr("CONFIRMED"));
}

TEST_F(ProductionControllerTest, ProcessCompletedJobs_InProgressJob_NoUpdate) {
    time_t future = time(nullptr) + 9999;
    line.enqueue({ "ORD-001", "S-001", 5, 9999, future });

    EXPECT_CALL(orderRepo, update(_)).Times(0);
    setup();
    ctrl->processCompletedJobs();

    EXPECT_FALSE(line.isEmpty());
}

TEST_F(ProductionControllerTest, ProcessCompletedJobs_MultipleCompleted_ProcessesAll) {
    time_t past = time(nullptr) - 9999;
    line.enqueue({ "ORD-001", "S-001", 5, 1, past });
    line.enqueue({ "ORD-002", "S-002", 3, 1, past });

    Order o1; o1.orderNo = "ORD-001"; o1.sampleId = "S-001"; o1.quantity = 3;
    o1.approve(false); o1.productionQty = 5;
    Order o2; o2.orderNo = "ORD-002"; o2.sampleId = "S-002"; o2.quantity = 2;
    o2.approve(false); o2.productionQty = 3;

    EXPECT_CALL(orderRepo, findById("ORD-001")).WillOnce(Return(o1));
    EXPECT_CALL(sampleRepo, findById("S-001")).WillOnce(Return(makeSample("S-001", 0)));
    EXPECT_CALL(orderRepo, findById("ORD-002")).WillOnce(Return(o2));
    EXPECT_CALL(sampleRepo, findById("S-002")).WillOnce(Return(makeSample("S-002", 0)));
    EXPECT_CALL(orderRepo, update(_)).Times(2);
    EXPECT_CALL(sampleRepo, update(_)).Times(2);

    setup();
    ctrl->processCompletedJobs();

    EXPECT_TRUE(line.isEmpty());
}
