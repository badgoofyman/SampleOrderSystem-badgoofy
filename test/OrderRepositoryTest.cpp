#include <gmock/gmock.h>
#include <cstdio>
#include <filesystem>
#include "../repository/OrderRepository.h"

namespace fs = std::filesystem;
static const std::string TEST_FILE = "data/test_orders_tmp.json";

class OrderRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        fs::create_directories("data");
        ::remove(TEST_FILE.c_str());
    }
    void TearDown() override {
        ::remove(TEST_FILE.c_str());
    }
    OrderRepository repo{TEST_FILE};

    static Order makeOrder(const std::string& no,
                           OrderStatus status = OrderStatus::RESERVED) {
        Order o;
        o.orderNo      = no;
        o.sampleId     = "S-001";
        o.customerName = "TestCo";
        o.quantity     = 10;
        switch (status) {
        case OrderStatus::PRODUCING: o.approve(false); break;
        case OrderStatus::CONFIRMED: o.approve(true);  break;
        case OrderStatus::RELEASE:   o.approve(true); o.release(); break;
        case OrderStatus::REJECTED:  o.reject();       break;
        default: break; // RESERVED = 기본값
        }
        return o;
    }
};

TEST_F(OrderRepositoryTest, FindAll_emptyFile_returnsEmptyVector) {
    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(OrderRepositoryTest, Save_thenFindAll_returnsSavedOrder) {
    repo.save(makeOrder("ORD-001"));
    auto all = repo.findAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].orderNo, "ORD-001");
    EXPECT_EQ(all[0].sampleId, "S-001");
    EXPECT_EQ(all[0].customerName, "TestCo");
    EXPECT_EQ(all[0].quantity, 10);
    EXPECT_EQ(all[0].getStatus(), OrderStatus::RESERVED);
}

TEST_F(OrderRepositoryTest, FindById_exists_returnsOrder) {
    repo.save(makeOrder("ORD-001"));
    auto result = repo.findById("ORD-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->orderNo, "ORD-001");
}

TEST_F(OrderRepositoryTest, FindById_notExists_returnsNullopt) {
    EXPECT_FALSE(repo.findById("NONE").has_value());
}

TEST_F(OrderRepositoryTest, Update_statusChange_persistsCorrectly) {
    Order o = makeOrder("ORD-001");
    repo.save(o);
    o.approve(false);
    repo.update(o);
    auto result = repo.findById("ORD-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getStatus(), OrderStatus::PRODUCING);
}

TEST_F(OrderRepositoryTest, ProductionStartTime_saveAndLoad_exactValue) {
    Order o = makeOrder("ORD-001");
    o.productionStartTime = 1749689400LL;
    repo.save(o);
    auto result = repo.findById("ORD-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->productionStartTime, 1749689400LL);
}

TEST_F(OrderRepositoryTest, ReleasedAt_saveAndLoad_exactValue) {
    Order o = makeOrder("ORD-001");
    o.releasedAt = 1749775800LL;
    repo.save(o);
    auto result = repo.findById("ORD-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->releasedAt, 1749775800LL);
}

TEST_F(OrderRepositoryTest, FindByStatus_RESERVED_returnsOnlyReserved) {
    repo.save(makeOrder("ORD-001", OrderStatus::RESERVED));
    repo.save(makeOrder("ORD-002", OrderStatus::PRODUCING));
    repo.save(makeOrder("ORD-003", OrderStatus::RESERVED));
    auto reserved = repo.findByStatus(OrderStatus::RESERVED);
    ASSERT_EQ(reserved.size(), 2u);
    for (const auto& o : reserved)
        EXPECT_EQ(o.getStatus(), OrderStatus::RESERVED);
}

TEST_F(OrderRepositoryTest, FindByStatus_PRODUCING_returnsOnlyProducing) {
    repo.save(makeOrder("ORD-001", OrderStatus::RESERVED));
    repo.save(makeOrder("ORD-002", OrderStatus::PRODUCING));
    auto producing = repo.findByStatus(OrderStatus::PRODUCING);
    ASSERT_EQ(producing.size(), 1u);
    EXPECT_EQ(producing[0].orderNo, "ORD-002");
}

TEST_F(OrderRepositoryTest, Remove_thenFindById_returnsNullopt) {
    repo.save(makeOrder("ORD-001"));
    repo.remove("ORD-001");
    EXPECT_FALSE(repo.findById("ORD-001").has_value());
}

TEST_F(OrderRepositoryTest, Persistence_reloadFromFile_sameData) {
    Order o = makeOrder("ORD-001", OrderStatus::CONFIRMED);
    o.productionStartTime = 1749689400LL;
    repo.save(o);
    OrderRepository repo2(TEST_FILE);
    auto result = repo2.findById("ORD-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getStatus(), OrderStatus::CONFIRMED);
    EXPECT_EQ(result->productionStartTime, 1749689400LL);
}
