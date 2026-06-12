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

// ===== placeOrder =====

TEST_F(OrderControllerTest, PlaceOrder_InvalidSampleId_PrintsError) {
    EXPECT_CALL(sampleRepo, findById("S-999")).WillRepeatedly(Return(std::nullopt));
    EXPECT_CALL(orderRepo, findAll()).WillRepeatedly(Return(std::vector<Order>{}));

    // S-999 입력 → 오류 출력 → EOF → 취소
    setup("S-999\n");
    ctrl->placeOrder();

    EXPECT_THAT(out.str(), HasSubstr("없습니다"));
}

TEST_F(OrderControllerTest, PlaceOrder_YesConfirm_SavesReservedOrder) {
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillOnce(Return(makeSample("S-001", 50)));
    EXPECT_CALL(orderRepo, findAll()).WillRepeatedly(Return(std::vector<Order>{}));
    EXPECT_CALL(orderRepo, save(_)).Times(1);

    // 시료 ID → 고객명 → 수량 → Y 확인
    setup("S-001\n고객A\n10\nY\n");
    ctrl->placeOrder();
}

TEST_F(OrderControllerTest, PlaceOrder_NoConfirm_DoesNotSave) {
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillOnce(Return(makeSample("S-001", 50)));
    EXPECT_CALL(orderRepo, findAll()).WillRepeatedly(Return(std::vector<Order>{}));
    EXPECT_CALL(orderRepo, save(_)).Times(0);

    // N 선택 → 취소
    setup("S-001\n고객A\n10\nN\n");
    ctrl->placeOrder();
}

TEST_F(OrderControllerTest, PlaceOrder_InvalidThenValid_Reprompts) {
    EXPECT_CALL(sampleRepo, findById("S-999")).WillOnce(Return(std::nullopt));
    EXPECT_CALL(sampleRepo, findById("S-001")).WillOnce(Return(makeSample("S-001", 50)));
    EXPECT_CALL(orderRepo, findAll()).WillRepeatedly(Return(std::vector<Order>{}));
    EXPECT_CALL(orderRepo, save(_)).Times(1);

    // S-999 오류 → S-001 정상 → 확인 Y
    setup("S-999\nS-001\n고객A\n10\nY\n");
    ctrl->placeOrder();

    EXPECT_THAT(out.str(), HasSubstr("없습니다"));
}

TEST_F(OrderControllerTest, PlaceOrder_EmptyCustomerName_Reprompts) {
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillRepeatedly(Return(makeSample("S-001", 50)));
    EXPECT_CALL(orderRepo, findAll()).WillRepeatedly(Return(std::vector<Order>{}));
    EXPECT_CALL(orderRepo, save(_)).Times(1);

    // 빈 고객명 → 오류 → 정상 고객명 → 수량 → Y
    setup("S-001\n\n고객A\n10\nY\n");
    ctrl->placeOrder();

    EXPECT_THAT(out.str(), HasSubstr("고객명"));
}

TEST_F(OrderControllerTest, PlaceOrder_ZeroQuantity_Reprompts) {
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillRepeatedly(Return(makeSample("S-001", 50)));
    EXPECT_CALL(orderRepo, findAll()).WillRepeatedly(Return(std::vector<Order>{}));
    EXPECT_CALL(orderRepo, save(_)).Times(1);

    // 수량 0 → 오류 → 정상 수량 → Y
    setup("S-001\n고객A\n0\n5\nY\n");
    ctrl->placeOrder();

    EXPECT_THAT(out.str(), HasSubstr("수량"));
}

// ===== processApproval =====

TEST_F(OrderControllerTest, ProcessApproval_EnoughStock_SetsConfirmed) {
    Order o;
    o.orderNo = "ORD-20260612-0001"; o.sampleId = "S-001";
    o.customerName = "고객A"; o.quantity = 10;

    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::RESERVED))
        .WillOnce(Return(std::vector<Order>{ o }));
    EXPECT_CALL(orderRepo, findAll())
        .WillOnce(Return(std::vector<Order>{ o }));
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillOnce(Return(makeSample("S-001", 50)));
    EXPECT_CALL(orderRepo, update(_)).Times(1);
    EXPECT_CALL(sampleRepo, update(_)).Times(1);

    setup("1\nY\n");  // 번호 1 선택, 승인
    ctrl->processApproval();

    EXPECT_THAT(out.str(), HasSubstr("CONFIRMED"));
}

TEST_F(OrderControllerTest, ProcessApproval_InsufficientStock_SetsProducing) {
    Order o;
    o.orderNo = "ORD-20260612-0002"; o.sampleId = "S-001";
    o.customerName = "고객B"; o.quantity = 30;

    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::RESERVED))
        .WillOnce(Return(std::vector<Order>{ o }));
    EXPECT_CALL(orderRepo, findAll())
        .WillOnce(Return(std::vector<Order>{ o }));
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillOnce(Return(makeSample("S-001", 10)));
    EXPECT_CALL(orderRepo, update(_)).Times(1);

    setup("1\nY\n");
    ctrl->processApproval();

    EXPECT_THAT(out.str(), HasSubstr("PRODUCING"));
    EXPECT_EQ(line.size(), 1u);
}

TEST_F(OrderControllerTest, ProcessApproval_Reject_SetsRejected) {
    Order o;
    o.orderNo = "ORD-20260612-0003"; o.sampleId = "S-001";
    o.customerName = "고객C"; o.quantity = 5;

    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::RESERVED))
        .WillOnce(Return(std::vector<Order>{ o }));
    EXPECT_CALL(orderRepo, findAll())
        .WillOnce(Return(std::vector<Order>{ o }));
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillOnce(Return(makeSample("S-001", 50)));
    EXPECT_CALL(orderRepo, update(_)).Times(1);

    setup("1\nN\n");  // 번호 1 선택, 거절
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

TEST_F(OrderControllerTest, ProcessApproval_FifoStock_AccountsForConfirmed) {
    // stock=15, 이미 CONFIRMED 주문 qty=10 → available=5 < 요청 qty=8 → PRODUCING
    Order confirmed_o;
    confirmed_o.orderNo = "ORD-OLD"; confirmed_o.sampleId = "S-001";
    confirmed_o.quantity = 10;
    confirmed_o.approve(true); // CONFIRMED

    Order reserved_o;
    reserved_o.orderNo = "ORD-NEW"; reserved_o.sampleId = "S-001";
    reserved_o.customerName = "고객A"; reserved_o.quantity = 8;

    EXPECT_CALL(orderRepo, findByStatus(OrderStatus::RESERVED))
        .WillOnce(Return(std::vector<Order>{ reserved_o }));
    EXPECT_CALL(orderRepo, findAll())
        .WillOnce(Return(std::vector<Order>{ confirmed_o, reserved_o }));
    EXPECT_CALL(sampleRepo, findById("S-001"))
        .WillOnce(Return(makeSample("S-001", 15)));
    EXPECT_CALL(orderRepo, update(_)).Times(1);

    setup("1\nY\n");
    ctrl->processApproval();

    EXPECT_THAT(out.str(), HasSubstr("PRODUCING"));
    EXPECT_EQ(line.size(), 1u);
}
