#include <gmock/gmock.h>
#include "MockRepositories.h"
#include "../controller/ReleaseController.h"
#include <sstream>

using namespace testing;

class ReleaseControllerTest : public Test {
protected:
    MockSampleRepository sampleRepo;
    MockOrderRepository  orderRepo;

    std::istringstream in;
    std::ostringstream out;

    std::unique_ptr<ReleaseController> ctrl;

    void setup(const std::string& input) {
        in.str(input);
        in.clear();
        ctrl = std::make_unique<ReleaseController>(orderRepo, in, out);
    }
};

TEST_F(ReleaseControllerTest, ProcessRelease_NoConfirmedOrders_PrintsMessage) {
    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::CONFIRMED))
        .WillOnce(Return(std::vector<Order>{}));

    setup("");
    ctrl->processRelease();

    EXPECT_THAT(out.str(), HasSubstr("없습니다"));
}

TEST_F(ReleaseControllerTest, ProcessRelease_ValidOrder_SetsReleaseAndUpdates) {
    Order o;
    o.orderNo = "ORD-001"; o.sampleId = "S-001"; o.customerName = "고객A"; o.quantity = 10;
    // CONFIRMED 상태로 설정
    o.approve(true);

    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::CONFIRMED))
        .WillOnce(Return(std::vector<Order>{ o }));
    EXPECT_CALL(orderRepo, update(_)).Times(1);

    setup("ORD-001\n");
    ctrl->processRelease();

    EXPECT_THAT(out.str(), HasSubstr("출고"));
}

TEST_F(ReleaseControllerTest, ProcessRelease_InvalidOrderNo_PrintsError) {
    Order o;
    o.orderNo = "ORD-001"; o.sampleId = "S-001"; o.customerName = "고객A"; o.quantity = 10;
    o.approve(true);

    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::CONFIRMED))
        .WillOnce(Return(std::vector<Order>{ o }));

    setup("ORD-INVALID\n");
    ctrl->processRelease();

    EXPECT_THAT(out.str(), HasSubstr("찾을 수 없습니다"));
}
