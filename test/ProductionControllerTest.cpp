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
        ctrl = std::make_unique<ProductionController>(sampleRepo, orderRepo, line, in, out);
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
