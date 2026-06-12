#pragma once
#include "IRepository.h"
#include "../model/Order.h"
#include <vector>

class IOrderRepository : public IRepository<Order> {
public:
    virtual std::vector<Order> findByStatus(OrderStatus status) const = 0;
};
