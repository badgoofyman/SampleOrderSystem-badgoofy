#include <gmock/gmock.h>
#include "../model/Sample.h"
#include "../model/Order.h"
#include "../model/ProductionLine.h"
#include "../util/ProductionCalculator.h"
#include <algorithm>
#include <optional>
#include <sstream>
#include <vector>

using namespace testing;

// 인메모리 저장소 — 통합 테스트 전용, Mock 없이 실제 데이터 흐름 검증
class InMemorySampleRepo {
public:
    std::vector<Sample> data;
    std::optional<Sample> findById(const std::string& id) const {
        for (const auto& s : data) if (s.id == id) return s;
        return std::nullopt;
    }
    void save(const Sample& s)   { data.push_back(s); }
    void update(const Sample& s) {
        for (auto& e : data) if (e.id == s.id) { e = s; return; }
    }
};

class InMemoryOrderRepo {
public:
    std::vector<Order> data;
    std::optional<Order> findById(const std::string& no) const {
        for (const auto& o : data) if (o.orderNo == no) return o;
        return std::nullopt;
    }
    void save(const Order& o)   { data.push_back(o); }
    void update(const Order& o) {
        for (auto& e : data) if (e.orderNo == o.orderNo) { e = o; return; }
    }
    std::vector<Order> findByStatus(OrderStatus s) const {
        std::vector<Order> result;
        for (const auto& o : data) if (o.getStatus() == s) result.push_back(o);
        return result;
    }
};

// ──────────────────────────────────────────────────────
// 통합 시나리오 1: RESERVED → CONFIRMED → RELEASE 전체 흐름
// ──────────────────────────────────────────────────────
TEST(IntegrationTest, FullOrderFlow_ReservedToRelease) {
    InMemorySampleRepo sampleRepo;
    InMemoryOrderRepo  orderRepo;

    // 시료 등록
    Sample s;
    s.id = "S-001"; s.name = "웨이퍼"; s.avgProductionTime = 60; s.yield = 0.9; s.stock = 100;
    sampleRepo.save(s);

    // 주문 접수 (RESERVED)
    Order o;
    o.orderNo = "ORD-001"; o.sampleId = "S-001"; o.customerName = "고객A"; o.quantity = 10;
    orderRepo.save(o);

    ASSERT_EQ(orderRepo.data[0].getStatus(), OrderStatus::RESERVED);

    // 재고 충분 승인 → CONFIRMED + 재고 차감
    auto sampleOpt = sampleRepo.findById("S-001");
    ASSERT_TRUE(sampleOpt);
    int available = sampleOpt->stock;  // 100 >= 10 → 충분

    ASSERT_GE(available, o.quantity);
    Order approved = orderRepo.data[0];
    approved.approve(true);  // CONFIRMED
    Sample updated = *sampleOpt;
    updated.stock -= approved.quantity;
    sampleRepo.update(updated);
    orderRepo.update(approved);

    EXPECT_EQ(orderRepo.data[0].getStatus(), OrderStatus::CONFIRMED);
    EXPECT_EQ(sampleRepo.data[0].stock, 90);

    // 출고 (RELEASE)
    Order released = orderRepo.data[0];
    released.release();
    released.releasedAt = 1234567890;
    orderRepo.update(released);

    EXPECT_EQ(orderRepo.data[0].getStatus(), OrderStatus::RELEASE);
    EXPECT_NE(orderRepo.data[0].releasedAt, 0);
}

// ──────────────────────────────────────────────────────
// 통합 시나리오 2: 재고 부족 → PRODUCING → 생산완료 → CONFIRMED
// ──────────────────────────────────────────────────────
TEST(IntegrationTest, InsufficientStock_ProducingFlow) {
    InMemorySampleRepo sampleRepo;
    InMemoryOrderRepo  orderRepo;
    ProductionLine     line;

    // 시료: 재고 5
    Sample s;
    s.id = "S-001"; s.name = "웨이퍼"; s.avgProductionTime = 60; s.yield = 0.9; s.stock = 5;
    sampleRepo.save(s);

    // 주문 수량 15 → 부족분 10
    Order o;
    o.orderNo = "ORD-001"; o.sampleId = "S-001"; o.customerName = "고객A"; o.quantity = 15;
    orderRepo.save(o);

    // 재고 부족 승인
    int shortage = o.quantity - s.stock;  // 10
    int prodQty  = ProductionCalculator::calcProductionQty(shortage, s.yield);  // ceil(10/(0.9*0.9)) = 13
    long long totalSec = ProductionCalculator::calcTotalTimeSeconds(s.avgProductionTime, prodQty);

    Order approvedOrder = orderRepo.data[0];
    approvedOrder.productionQty       = prodQty;
    approvedOrder.productionStartTime = 1000;
    approvedOrder.approve(false);  // PRODUCING
    line.enqueue({ approvedOrder.orderNo, approvedOrder.sampleId, prodQty, totalSec, 1000 });
    orderRepo.update(approvedOrder);

    EXPECT_EQ(orderRepo.data[0].getStatus(), OrderStatus::PRODUCING);
    EXPECT_EQ(line.size(), 1u);

    // 생산 완료 시뮬레이션: completeProduction() 호출
    Order completed = orderRepo.data[0];
    completed.completeProduction();  // PRODUCING → CONFIRMED
    Sample updatedSample = sampleRepo.data[0];
    updatedSample.stock += completed.productionQty - completed.quantity;
    sampleRepo.update(updatedSample);
    orderRepo.update(completed);
    line.completeAndAdvance();

    EXPECT_EQ(orderRepo.data[0].getStatus(), OrderStatus::CONFIRMED);
    EXPECT_TRUE(line.isEmpty());
    // stock = 5 + (prodQty - 15) — 생산 후 잉여 재고
    EXPECT_EQ(sampleRepo.data[0].stock, 5 + prodQty - 15);
}

// ──────────────────────────────────────────────────────
// 통합 시나리오 3: ProductionLine 재구성 — startTime 순서 정렬
// ──────────────────────────────────────────────────────
TEST(IntegrationTest, ProductionLineRestore_SortedByStartTime) {
    InMemoryOrderRepo orderRepo;
    InMemorySampleRepo sampleRepo;

    Sample s;
    s.id = "S-001"; s.name = "웨이퍼"; s.avgProductionTime = 60; s.yield = 0.9; s.stock = 0;
    sampleRepo.save(s);

    // 나중에 시작한 주문이 먼저 저장됨 (정렬 필요)
    Order o2;
    o2.orderNo = "ORD-002"; o2.sampleId = "S-001"; o2.quantity = 5;
    o2.approve(false); o2.productionQty = 6; o2.productionStartTime = 2000;
    orderRepo.save(o2);

    Order o1;
    o1.orderNo = "ORD-001"; o1.sampleId = "S-001"; o1.quantity = 3;
    o1.approve(false); o1.productionQty = 4; o1.productionStartTime = 1000;
    orderRepo.save(o1);

    // buildProductionLine 로직 재현
    ProductionLine line;
    auto producing = orderRepo.findByStatus(OrderStatus::PRODUCING);
    std::sort(producing.begin(), producing.end(),
        [](const Order& a, const Order& b) {
            return a.productionStartTime < b.productionStartTime;
        });
    for (const auto& o : producing) {
        auto sOpt = sampleRepo.findById(o.sampleId);
        if (!sOpt) continue;
        long long sec = ProductionCalculator::calcTotalTimeSeconds(sOpt->avgProductionTime, o.productionQty);
        line.enqueue({ o.orderNo, o.sampleId, o.productionQty, sec, o.productionStartTime });
    }

    // startTime이 빠른 ORD-001이 first여야 함
    EXPECT_EQ(line.size(), 2u);
    EXPECT_EQ(line.current().orderNo, "ORD-001");
}

// ──────────────────────────────────────────────────────
// 통합 시나리오 4: REJECTED 주문은 findByStatus(RESERVED)에 나오지 않음
// ──────────────────────────────────────────────────────
TEST(IntegrationTest, RejectedOrder_NotInReservedList) {
    InMemoryOrderRepo orderRepo;

    Order o1; o1.orderNo = "ORD-001"; o1.sampleId = "S-001"; o1.quantity = 5;
    Order o2; o2.orderNo = "ORD-002"; o2.sampleId = "S-001"; o2.quantity = 3;
    o2.reject();  // REJECTED
    orderRepo.save(o1);
    orderRepo.save(o2);

    auto reserved = orderRepo.findByStatus(OrderStatus::RESERVED);
    EXPECT_EQ(reserved.size(), 1u);
    EXPECT_EQ(reserved[0].orderNo, "ORD-001");
}
