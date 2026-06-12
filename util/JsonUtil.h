#pragma once
#include <string>
#include <vector>
#include <map>

namespace JsonUtil {

enum class ValueType { String, Number };

struct FieldDef {
    std::string name;
    ValueType   type;
};

// JSON 오브젝트를 string->string 맵으로 표현 (숫자도 문자열로 저장)
using JsonObject = std::map<std::string, std::string>;

// JSON 배열 파일 읽기. 파일 없으면 빈 벡터 반환.
std::vector<JsonObject> readArray(const std::string& filePath);

// JSON 배열 파일 쓰기. fieldDefs 순서대로 출력, 숫자는 따옴표 없이 출력.
void writeArray(const std::string& filePath,
                const std::vector<JsonObject>& objects,
                const std::vector<FieldDef>& fieldDefs);

} // namespace JsonUtil
