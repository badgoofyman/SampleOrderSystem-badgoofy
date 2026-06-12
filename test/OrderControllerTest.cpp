#include <gmock/gmock.h>
#include "MockRepositories.h"
#include "../controller/OrderController.h"
#include "../model/ProductionLine.h"
#include <sstream>

using namespace testing;

class OrderControllerTest : public Test {
protected:
    MockSampleRepository sampleRepo;
    MockOrderRepository  orderRepo;
    ProductionLine       line;

    std::istringstream in;
    std::ostringstream out;

    std::unique_ptr<OrderController> ctrl;

    void setup(const std::string& input) {
        in.str(input);
        in.clear();
        ctrl = std::make_unique<OrderController>(sampleRepo, orderRepo, line, in, out);
    }

    Sample makeSample(const std::string& id, int stock, double yield = 0.9) {
        Sample s;
        s.id = id; s.name = "테스트 시료"; s.avgProductionTime = 60; s.yield = yield; s.stock = stock;
        return s;
    }
};

TEST_F(OrderControllerTest, PlaceOrder_InvalidSampleId_PrintsError) {
    EXPECT_CALL(sampleRepo, findById("S-999")).WillOnce(Return(std::nullopt));
    EXPECT_CALL(orderRepo, findAll()).WillRepeatedly(Return(std::vector<Order>{}));

    setup("S-999\n고객A\n10\n");
    ctrl->placeOrder();

    EXPECT_THAT(out.str(), HasSubstr("없습니다"));
}

TEST_F(OrderControllerTest, PlaceOrder_ValidSample_SavesReservedOrder) {
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillOnce(Return(makeSample("S-001", 50)));
    EXPECT_CALL(orderRepo, findAll()).WillRepeatedly(Return(std::vector<Order>{}));
    EXPECT_CALL(orderRepo, save(_)).Times(1);

    setup("S-001\n고객A\n10\n");
    ctrl->placeOrder();
}

TEST_F(OrderControllerTest, ProcessApproval_EnoughStock_SetsConfirmed) {
    Order o;
    o.orderNo = "ORD-20260612-0001"; o.sampleId = "S-001"; o.customerName = "고객A"; o.quantity = 10;

    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::RESERVED))
        .WillOnce(Return(std::vector<Order>{ o }));
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillOnce(Return(makeSample("S-001", 50)));
    EXPECT_CALL(orderRepo, update(_)).Times(1);
    EXPECT_CALL(sampleRepo, update(_)).Times(1);

    setup("ORD-20260612-0001\n1\n");
    ctrl->processApproval();

    EXPECT_THAT(out.str(), HasSubstr("CONFIRMED"));
}

TEST_F(OrderControllerTest, ProcessApproval_InsufficientStock_SetsProducing) {
    Order o;
    o.orderNo = "ORD-20260612-0002"; o.sampleId = "S-001"; o.customerName = "고객B"; o.quantity = 30;

    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::RESERVED))
        .WillOnce(Return(std::vector<Order>{ o }));
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillOnce(Return(makeSample("S-001", 10)));
    EXPECT_CALL(orderRepo, update(_)).Times(1);

    setup("ORD-20260612-0002\n1\n");
    ctrl->processApproval();

    EXPECT_THAT(out.str(), HasSubstr("PRODUCING"));
    EXPECT_EQ(line.size(), 1u);
}

TEST_F(OrderControllerTest, ProcessApproval_Reject_SetsRejected) {
    Order o;
    o.orderNo = "ORD-20260612-0003"; o.sampleId = "S-001"; o.customerName = "고객C"; o.quantity = 5;

    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::RESERVED))
        .WillOnce(Return(std::vector<Order>{ o }));
    EXPECT_CALL(orderRepo, update(_)).Times(1);

    setup("ORD-20260612-0003\n0\n");
    ctrl->processApproval();

    EXPECT_THAT(out.str(), HasSubstr("거절"));
}

TEST_F(OrderControllerTest, ProcessApproval_NoReservedOrders_PrintsMessage) {
    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::RESERVED))
        .WillOnce(Return(std::vector<Order>{}));

    setup("");
    ctrl->processApproval();

    EXPECT_THAT(out.str(), HasSubstr("없습니다"));
}
