#include "ReleaseController.h"
#include "../view/OrderView.h"
#include <ctime>
#include <ostream>

ReleaseController::ReleaseController(IOrderRepository& orderRepo,
                                     std::istream&     in,
                                     std::ostream&     out)
    : orderRepo_(orderRepo), in_(in), out_(out) {}

void ReleaseController::processRelease() {
    auto confirmed = orderRepo_.findByStatus(OrderStatus::CONFIRMED);
    if (confirmed.empty()) {
        out_ << "출고 대기 중인 주문이 없습니다.\n";
        return;
    }

    OrderView::printReservedList(confirmed, out_);  // 같은 테이블 형식 재사용
    std::string orderNo = OrderView::selectOrderNo(in_, out_);

    Order* target = nullptr;
    for (auto& o : confirmed)
        if (o.orderNo == orderNo) { target = &o; break; }

    if (!target) {
        OrderView::printInvalidOrderNo(out_);
        return;
    }

    target->release();
    target->releasedAt = time(nullptr);
    orderRepo_.update(*target);
    out_ << "출고 처리 완료: [" << target->orderNo << "] → RELEASE\n";
}
