#include "OrderController.h"
#include "../view/OrderView.h"
#include "../util/ProductionCalculator.h"
#include <ctime>
#include <iomanip>
#include <sstream>

OrderController::OrderController(IRepository<Sample>& sampleRepo,
                                 IOrderRepository&    orderRepo,
                                 ProductionLine&      line,
                                 std::istream&        in,
                                 std::ostream&        out)
    : sampleRepo_(sampleRepo), orderRepo_(orderRepo), line_(line), in_(in), out_(out) {}

std::string OrderController::generateOrderNo() const {
    time_t now = time(nullptr);
    struct tm tm_info;
#if defined(_WIN32)
    localtime_s(&tm_info, &now);
#else
    localtime_r(&now, &tm_info);
#endif
    char dateBuf[16] = {};
    strftime(dateBuf, sizeof(dateBuf), "%Y%m%d", &tm_info);

    auto all = orderRepo_.findAll();
    // 오늘 날짜 접두사와 일치하는 주문번호 중 가장 큰 시퀀스 번호 찾기
    std::string prefix = std::string("ORD-") + dateBuf + "-";
    int maxSeq = 0;
    for (const auto& o : all) {
        if (o.orderNo.substr(0, prefix.size()) == prefix) {
            try {
                int seq = std::stoi(o.orderNo.substr(prefix.size()));
                if (seq > maxSeq) maxSeq = seq;
            } catch (...) {}
        }
    }
    std::ostringstream oss;
    oss << prefix << std::setfill('0') << std::setw(4) << (maxSeq + 1);
    return oss.str();
}

void OrderController::placeOrder() {
    Order o = OrderView::inputNewOrder(in_, out_);

    auto sample = sampleRepo_.findById(o.sampleId);
    if (!sample) {
        out_ << "존재하지 않는 시료 ID입니다.\n";
        return;
    }
    o.orderNo = generateOrderNo();
    orderRepo_.save(o);
    OrderView::printOrderPlaced(o, out_);
}

void OrderController::approveOrder(Order& o, const Sample& s) {
    bool enough = s.stock >= o.quantity;
    if (enough) {
        o.approve(true);
        Sample updated = s;
        updated.stock -= o.quantity;
        sampleRepo_.update(updated);
    } else {
        int shortage = o.quantity - s.stock;
        int prodQty  = ProductionCalculator::calcProductionQty(shortage, s.yield);
        long long totalSec = ProductionCalculator::calcTotalTimeSeconds(s.avgProductionTime, prodQty);

        time_t startTime;
        if (line_.isEmpty()) {
            startTime = time(nullptr);
        } else {
            const auto& last = line_.jobs().back();
            startTime = last.startTime + static_cast<time_t>(last.totalTimeSeconds);
        }

        o.productionQty       = prodQty;
        o.productionStartTime = startTime;
        o.approve(false);

        line_.enqueue({ o.orderNo, o.sampleId, prodQty, totalSec, startTime });
    }
    orderRepo_.update(o);
}

void OrderController::processApproval() {
    auto reserved = orderRepo_.findByStatus(OrderStatus::RESERVED);
    if (reserved.empty()) {
        OrderView::printNoReservedOrders(out_);
        return;
    }
    OrderView::printReservedList(reserved, out_);

    std::string orderNo = OrderView::selectOrderNo(in_, out_);
    Order* target = nullptr;
    for (auto& o : reserved)
        if (o.orderNo == orderNo) { target = &o; break; }

    if (!target) {
        OrderView::printInvalidOrderNo(out_);
        return;
    }

    bool approve = OrderView::askApprove(in_, out_);
    if (approve) {
        auto sample = sampleRepo_.findById(target->sampleId);
        if (!sample) {
            out_ << "시료 정보를 찾을 수 없습니다.\n";
            return;
        }
        approveOrder(*target, *sample);
        OrderView::printApproved(*target, out_);
    } else {
        target->reject();
        orderRepo_.update(*target);
        OrderView::printRejected(*target, out_);
    }
}
