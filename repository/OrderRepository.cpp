#include "OrderRepository.h"
#include "../util/JsonUtil.h"
#include <algorithm>

static const std::vector<JsonUtil::FieldDef> ORDER_FIELDS = {
    { "orderNo",             JsonUtil::ValueType::String },
    { "sampleId",            JsonUtil::ValueType::String },
    { "customerName",        JsonUtil::ValueType::String },
    { "quantity",            JsonUtil::ValueType::Number },
    { "status",              JsonUtil::ValueType::String },
    { "productionStartTime", JsonUtil::ValueType::Number },
    { "releasedAt",          JsonUtil::ValueType::Number },
};

static std::string statusToString(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::RELEASE:   return "RELEASE";
    case OrderStatus::REJECTED:  return "REJECTED";
    default:                     return "RESERVED";
    }
}

static OrderStatus stringToStatus(const std::string& s) {
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASE")   return OrderStatus::RELEASE;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    return OrderStatus::RESERVED;
}

static Order rowToOrder(const JsonUtil::JsonObject& row) {
    auto get = [&](const std::string& k) -> const std::string& {
        static const std::string empty;
        auto it = row.find(k);
        return it != row.end() ? it->second : empty;
    };
    Order o;
    o.orderNo             = get("orderNo");
    o.sampleId            = get("sampleId");
    o.customerName        = get("customerName");
    o.quantity            = get("quantity").empty()            ? 0 : std::stoi(get("quantity"));
    o.productionStartTime = get("productionStartTime").empty() ? 0 : static_cast<time_t>(std::stoll(get("productionStartTime")));
    o.releasedAt          = get("releasedAt").empty()          ? 0 : static_cast<time_t>(std::stoll(get("releasedAt")));
    o.setStatusDirect(stringToStatus(get("status")));
    return o;
}

static JsonUtil::JsonObject orderToRow(const Order& o) {
    return {
        { "orderNo",             o.orderNo },
        { "sampleId",            o.sampleId },
        { "customerName",        o.customerName },
        { "quantity",            std::to_string(o.quantity) },
        { "status",              statusToString(o.getStatus()) },
        { "productionStartTime", std::to_string(static_cast<long long>(o.productionStartTime)) },
        { "releasedAt",          std::to_string(static_cast<long long>(o.releasedAt)) },
    };
}

OrderRepository::OrderRepository(const std::string& filePath)
    : filePath_(filePath) {}

std::vector<Order> OrderRepository::findAll() const {
    auto rows = JsonUtil::readArray(filePath_);
    std::vector<Order> result;
    result.reserve(rows.size());
    for (const auto& row : rows)
        result.push_back(rowToOrder(row));
    return result;
}

std::optional<Order> OrderRepository::findById(const std::string& id) const {
    for (const auto& o : findAll())
        if (o.orderNo == id) return o;
    return std::nullopt;
}

void OrderRepository::save(const Order& order) {
    auto rows = JsonUtil::readArray(filePath_);
    rows.push_back(orderToRow(order));
    JsonUtil::writeArray(filePath_, rows, ORDER_FIELDS);
}

void OrderRepository::update(const Order& order) {
    auto rows = JsonUtil::readArray(filePath_);
    for (auto& row : rows)
        if (row.count("orderNo") && row.at("orderNo") == order.orderNo)
            row = orderToRow(order);
    JsonUtil::writeArray(filePath_, rows, ORDER_FIELDS);
}

void OrderRepository::remove(const std::string& id) {
    auto rows = JsonUtil::readArray(filePath_);
    rows.erase(std::remove_if(rows.begin(), rows.end(),
        [&](const JsonUtil::JsonObject& r) {
            return r.count("orderNo") && r.at("orderNo") == id;
        }), rows.end());
    JsonUtil::writeArray(filePath_, rows, ORDER_FIELDS);
}

std::vector<Order> OrderRepository::findByStatus(OrderStatus status) const {
    std::vector<Order> result;
    for (const auto& o : findAll())
        if (o.getStatus() == status)
            result.push_back(o);
    return result;
}
