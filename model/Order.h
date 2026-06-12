#pragma once
#include <string>
#include <ctime>

enum class OrderStatus {
    RESERVED,
    PRODUCING,
    CONFIRMED,
    RELEASE,
    REJECTED
};

class Order {
public:
    std::string orderNo;
    std::string sampleId;
    std::string customerName;
    int         quantity            = 0;
    time_t      productionStartTime = 0;  // epoch, 0 = 미사용
    time_t      releasedAt          = 0;  // epoch, 0 = 미사용

    Order() = default;

    OrderStatus getStatus() const { return status_; }
    void        setStatusDirect(OrderStatus s) { status_ = s; } // 역직렬화 전용

    void approve(bool hasEnoughStock);
    void reject();
    void completeProduction();
    void release();

private:
    OrderStatus status_ = OrderStatus::RESERVED;
    void        transitionTo(OrderStatus next);
};
