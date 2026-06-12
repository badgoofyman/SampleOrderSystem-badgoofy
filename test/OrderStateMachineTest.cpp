#include <gmock/gmock.h>
#include "../model/Order.h"

// ---- 합법 전환 5가지 ----

TEST(OrderStateMachineTest, Approve_stockSufficient_RESERVED_to_CONFIRMED) {
    Order o;
    o.approve(true);
    EXPECT_EQ(o.getStatus(), OrderStatus::CONFIRMED);
}

TEST(OrderStateMachineTest, Approve_stockInsufficient_RESERVED_to_PRODUCING) {
    Order o;
    o.approve(false);
    EXPECT_EQ(o.getStatus(), OrderStatus::PRODUCING);
}

TEST(OrderStateMachineTest, Reject_RESERVED_to_REJECTED) {
    Order o;
    o.reject();
    EXPECT_EQ(o.getStatus(), OrderStatus::REJECTED);
}

TEST(OrderStateMachineTest, CompleteProduction_PRODUCING_to_CONFIRMED) {
    Order o;
    o.approve(false);
    o.completeProduction();
    EXPECT_EQ(o.getStatus(), OrderStatus::CONFIRMED);
}

TEST(OrderStateMachineTest, Release_CONFIRMED_to_RELEASE) {
    Order o;
    o.approve(true);
    o.release();
    EXPECT_EQ(o.getStatus(), OrderStatus::RELEASE);
}

// ---- 불법 전환 (std::logic_error) ----

TEST(OrderStateMachineTest, Illegal_RESERVED_to_RELEASE_throws) {
    Order o;
    EXPECT_THROW(o.release(), std::logic_error);
}

TEST(OrderStateMachineTest, Illegal_CONFIRMED_to_PRODUCING_throws) {
    Order o;
    o.approve(true);
    EXPECT_THROW(o.approve(false), std::logic_error);
}

TEST(OrderStateMachineTest, Illegal_RELEASE_to_CONFIRMED_throws) {
    Order o;
    o.approve(true);
    o.release();
    EXPECT_THROW(o.completeProduction(), std::logic_error);
}

TEST(OrderStateMachineTest, Illegal_REJECTED_to_RESERVED_throws) {
    Order o;
    o.reject();
    EXPECT_THROW(o.approve(true), std::logic_error);
}

TEST(OrderStateMachineTest, Illegal_PRODUCING_to_REJECTED_throws) {
    Order o;
    o.approve(false);
    EXPECT_THROW(o.reject(), std::logic_error);
}

// ---- 종단 상태에서 모든 전환 불가 ----

TEST(OrderStateMachineTest, RELEASE_is_terminal_no_transitions) {
    Order o;
    o.approve(true);
    o.release();
    EXPECT_THROW(o.approve(true),         std::logic_error);
    EXPECT_THROW(o.reject(),              std::logic_error);
    EXPECT_THROW(o.completeProduction(),  std::logic_error);
    EXPECT_THROW(o.release(),             std::logic_error);
}

TEST(OrderStateMachineTest, REJECTED_is_terminal_no_transitions) {
    Order o;
    o.reject();
    EXPECT_THROW(o.approve(true),         std::logic_error);
    EXPECT_THROW(o.reject(),              std::logic_error);
    EXPECT_THROW(o.completeProduction(),  std::logic_error);
    EXPECT_THROW(o.release(),             std::logic_error);
}
