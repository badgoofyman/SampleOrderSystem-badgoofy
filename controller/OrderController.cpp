#include "OrderController.h"
#include "../view/OrderView.h"
#include "../util/ProductionCalculator.h"
#include <ctime>
#include <iomanip>
#include <ostream>
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
    std::string prefix = std::string("ORD-") + dateBuf + "-";
    int maxSeq = 0;
    for (const auto& o : all) {
        if (o.orderNo.size() > prefix.size() &&
            o.orderNo.substr(0, prefix.size()) == prefix) {
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
    out_ << "\n  ── 시료 주문 ──\n";

    // 시료 ID 유효성 검사 루프
    std::string sampleId;
    std::optional<Sample> sample;
    while (true) {
        sampleId = OrderView::inputOrderSampleId(in_, out_);
        if (sampleId.empty()) return;
        sample = sampleRepo_.findById(sampleId);
        if (sample) break;
        OrderView::printInvalidSampleId(out_);
    }

    // 고객명 빈값 재프롬프트
    std::string customer;
    while (true) {
        customer = OrderView::inputCustomerName(in_, out_);
        if (!in_) return;
        if (!customer.empty()) break;
        OrderView::printInvalidCustomerName(out_);
    }

    // 수량 0 이하 재프롬프트
    int qty;
    while (true) {
        qty = OrderView::inputQuantity(in_, out_);
        if (!in_) return;
        if (qty > 0) break;
        OrderView::printInvalidQuantity(out_);
    }

    // 확인 화면
    OrderView::printOrderConfirm(*sample, customer, qty, out_);
    if (!OrderView::askYesNo(in_, out_)) {
        OrderView::printOrderCancelled(out_);
        return;
    }

    Order o;
    o.sampleId     = sampleId;
    o.customerName = customer;
    o.quantity     = qty;
    o.orderNo      = generateOrderNo();
    orderRepo_.save(o);
    OrderView::printOrderPlaced(o, out_);
}

void OrderController::approveOrder(Order& o, const Sample& s, int available) {
    bool enough = (available >= o.quantity);
    if (enough) {
        o.approve(true);
        Sample updated = s;
        updated.stock -= o.quantity;
        sampleRepo_.update(updated);
    } else {
        int shortage = o.quantity - available;
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

    OrderView::printOrderListNumbered(reserved, out_);
    int idx = OrderView::selectOrderIndex(static_cast<int>(reserved.size()), in_, out_);
    if (idx < 0) return;

    Order& target = reserved[idx];
    auto sample = sampleRepo_.findById(target.sampleId);
    if (!sample) {
        out_ << "  시료 정보를 찾을 수 없습니다.\n";
        return;
    }

    // 가용 재고 계산:
    // - CONFIRMED 경로: approveOrder에서 stock -= qty 즉시 차감 → stock에 이미 반영됨
    // - PRODUCING 경로: stock 차감 없음 → 별도 차감 필요
    int producingQty = 0;
    auto allOrders = orderRepo_.findAll();
    for (const auto& o : allOrders) {
        if (o.sampleId == target.sampleId && o.getStatus() == OrderStatus::PRODUCING)
            producingQty += o.quantity;
    }
    int available = sample->stock - producingQty;

    // 재고 충분 여부 표시 후 Y/N 선택
    int shortage = 0, prodQty = 0, totalMin = 0;
    if (available >= target.quantity) {
        OrderView::printStockCheckSufficient(*sample, target.quantity, out_);
    } else {
        shortage = target.quantity - available;
        prodQty  = ProductionCalculator::calcProductionQty(shortage, sample->yield);
        totalMin = ProductionCalculator::calcTotalTimeMinutes(sample->avgProductionTime, prodQty);
        OrderView::printStockCheckInsufficient(*sample, target.quantity, available,
                                               shortage, prodQty, totalMin, out_);
    }

    bool approve = OrderView::askYesNo(in_, out_);
    if (approve) {
        approveOrder(target, *sample, available);
        OrderView::printApprovalResult(target, out_);
    } else {
        target.reject();
        orderRepo_.update(target);
        OrderView::printRejectionResult(target, out_);
    }
}
