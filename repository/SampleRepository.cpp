#include "SampleRepository.h"
#include "../util/JsonUtil.h"
#include <algorithm>
#include <sstream>

static const std::vector<JsonUtil::FieldDef> SAMPLE_FIELDS = {
    { "id",               JsonUtil::ValueType::String },
    { "name",             JsonUtil::ValueType::String },
    { "avgProductionTime",JsonUtil::ValueType::Number },
    { "yield",            JsonUtil::ValueType::Number },
    { "stock",            JsonUtil::ValueType::Number },
};

static std::string fmtDouble(double d) {
    std::ostringstream oss;
    oss << d;
    return oss.str();
}

static Sample rowToSample(const JsonUtil::JsonObject& row) {
    Sample s;
    auto get = [&](const std::string& k) -> const std::string& {
        static const std::string empty;
        auto it = row.find(k);
        return it != row.end() ? it->second : empty;
    };
    s.id                = get("id");
    s.name              = get("name");
    s.avgProductionTime = get("avgProductionTime").empty() ? 0 : std::stoi(get("avgProductionTime"));
    s.yield             = get("yield").empty() ? 0.0 : std::stod(get("yield"));
    s.stock             = get("stock").empty() ? 0 : std::stoi(get("stock"));
    return s;
}

static JsonUtil::JsonObject sampleToRow(const Sample& s) {
    return {
        { "id",               s.id },
        { "name",             s.name },
        { "avgProductionTime",std::to_string(s.avgProductionTime) },
        { "yield",            fmtDouble(s.yield) },
        { "stock",            std::to_string(s.stock) },
    };
}

SampleRepository::SampleRepository(const std::string& filePath)
    : filePath_(filePath) {}

std::vector<Sample> SampleRepository::findAll() const {
    auto rows = JsonUtil::readArray(filePath_);
    std::vector<Sample> result;
    result.reserve(rows.size());
    for (const auto& row : rows)
        result.push_back(rowToSample(row));
    return result;
}

std::optional<Sample> SampleRepository::findById(const std::string& id) const {
    for (const auto& s : findAll())
        if (s.id == id) return s;
    return std::nullopt;
}

void SampleRepository::save(const Sample& sample) {
    auto rows = JsonUtil::readArray(filePath_);
    rows.push_back(sampleToRow(sample));
    JsonUtil::writeArray(filePath_, rows, SAMPLE_FIELDS);
}

void SampleRepository::update(const Sample& sample) {
    auto rows = JsonUtil::readArray(filePath_);
    for (auto& row : rows)
        if (row.count("id") && row.at("id") == sample.id)
            row = sampleToRow(sample);
    JsonUtil::writeArray(filePath_, rows, SAMPLE_FIELDS);
}

void SampleRepository::remove(const std::string& id) {
    auto rows = JsonUtil::readArray(filePath_);
    rows.erase(std::remove_if(rows.begin(), rows.end(),
        [&](const JsonUtil::JsonObject& r) {
            return r.count("id") && r.at("id") == id;
        }), rows.end());
    JsonUtil::writeArray(filePath_, rows, SAMPLE_FIELDS);
}
