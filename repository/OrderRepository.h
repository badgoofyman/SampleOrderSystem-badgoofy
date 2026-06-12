#pragma once
#include "IOrderRepository.h"
#include "../model/Order.h"
#include "../util/JsonUtil.h"
#include <string>
#include <vector>

class OrderRepository : public IOrderRepository {
public:
    explicit OrderRepository(const std::string& filePath = "data/orders.json");

    std::vector<Order>   findAll()                          const override;
    std::optional<Order> findById(const std::string& id)   const override;
    void                 save(const Order& order)                 override;
    void                 update(const Order& order)               override;
    void                 remove(const std::string& id)            override;
    std::vector<Order>   findByStatus(OrderStatus status)   const override;

private:
    std::string filePath_;

    static Order              rowToOrder(const JsonUtil::JsonObject& row);
    static JsonUtil::JsonObject orderToRow(const Order& o);
};
