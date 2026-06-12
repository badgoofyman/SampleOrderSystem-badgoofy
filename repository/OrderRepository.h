#pragma once
#include "IRepository.h"
#include "../model/Order.h"
#include <string>
#include <vector>

class OrderRepository : public IRepository<Order> {
public:
    explicit OrderRepository(const std::string& filePath = "data/orders.json");

    std::vector<Order>   findAll()                          const override;
    std::optional<Order> findById(const std::string& id)   const override;
    void                 save(const Order& order)                 override;
    void                 update(const Order& order)               override;
    void                 remove(const std::string& id)            override;

    // IRepository 확장: 상태별 조회
    std::vector<Order>   findByStatus(OrderStatus status)  const;

private:
    std::string filePath_;
};
