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
        OrderView::printNoConfirmedOrders(out_);
        return;
    }

    OrderView::printOrderList(confirmed, out_);

    int idx = OrderView::selectOrderIndex(static_cast<int>(confirmed.size()), in_, out_);
    if (idx < 0) return;

    Order& target = confirmed[idx];
    target.release();
    target.releasedAt = time(nullptr);
    orderRepo_.update(target);
    OrderView::printReleaseResult(target, target.releasedAt, out_);
}
