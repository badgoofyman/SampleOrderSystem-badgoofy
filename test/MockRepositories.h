#pragma once
#include <gmock/gmock.h>
#include "../repository/IRepository.h"
#include "../repository/IOrderRepository.h"
#include "../model/Sample.h"
#include "../model/Order.h"

class MockSampleRepository : public IRepository<Sample> {
public:
    MOCK_METHOD(std::vector<Sample>,    findAll,  (), (const, override));
    MOCK_METHOD(std::optional<Sample>,  findById, (const std::string&), (const, override));
    MOCK_METHOD(void,                   save,     (const Sample&), (override));
    MOCK_METHOD(void,                   update,   (const Sample&), (override));
    MOCK_METHOD(void,                   remove,   (const std::string&), (override));
};

class MockOrderRepository : public IOrderRepository {
public:
    MOCK_METHOD(std::vector<Order>,    findAll,      (), (const, override));
    MOCK_METHOD(std::optional<Order>,  findById,     (const std::string&), (const, override));
    MOCK_METHOD(void,                  save,         (const Order&), (override));
    MOCK_METHOD(void,                  update,       (const Order&), (override));
    MOCK_METHOD(void,                  remove,       (const std::string&), (override));
    MOCK_METHOD(std::vector<Order>,    findByStatus, (OrderStatus), (const, override));
};
