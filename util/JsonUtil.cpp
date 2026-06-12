#include "JsonUtil.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

namespace JsonUtil {

// ---- 파서 내부 헬퍼 ----

static void skipWs(const std::string& s, size_t& pos) {
    while (pos < s.size() &&
           (s[pos] == ' ' || s[pos] == '\n' || s[pos] == '\r' || s[pos] == '\t'))
        ++pos;
}

static std::string parseString(const std::string& s, size_t& pos) {
    ++pos; // opening '"'
    std::string result;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\' && pos + 1 < s.size()) {
            ++pos;
            switch (s[pos]) {
            case '"':  result += '"';  break;
            case '\\': result += '\\'; break;
            case 'n':  result += '\n'; break;
            case 'r':  result += '\r'; break;
            case 't':  result += '\t'; break;
            default:   result += s[pos]; break;
            }
        } else {
            result += s[pos];
        }
        ++pos;
    }
    if (pos < s.size()) ++pos; // closing '"'
    return result;
}

static std::string parseValue(const std::string& s, size_t& pos) {
    skipWs(s, pos);
    if (pos >= s.size()) return "";
    if (s[pos] == '"') return parseString(s, pos);
    std::string result;
    while (pos < s.size() &&
           s[pos] != ',' && s[pos] != '}' && s[pos] != ']' &&
           s[pos] != ' '  && s[pos] != '\n' && s[pos] != '\r' && s[pos] != '\t')
        result += s[pos++];
    return result;
}

static std::string escapeString(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:   out += c;      break;
        }
    }
    return out;
}

// ---- 공개 API ----

std::vector<JsonObject> readArray(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        // 파일이 없으면 빈 배열 (정상 — 첫 실행)
        // 파일이 있는데 열기 실패 → 권한/잠금 오류로 데이터 손실 방지
        if (std::filesystem::exists(filePath))
            throw std::runtime_error("Cannot open file (permission/lock?): " + filePath);
        return {};
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    std::vector<JsonObject> result;
    size_t pos = 0;

    skipWs(content, pos);
    if (pos >= content.size() || content[pos] != '[') return {};
    ++pos;

    while (pos < content.size()) {
        skipWs(content, pos);
        if (pos >= content.size() || content[pos] == ']') break;
        if (content[pos] == ',') { ++pos; continue; }
        if (content[pos] != '{') { ++pos; continue; }
        ++pos;

        JsonObject obj;
        while (pos < content.size()) {
            skipWs(content, pos);
            if (pos >= content.size() || content[pos] == '}') { ++pos; break; }
            if (content[pos] == ',') { ++pos; continue; }
            if (content[pos] != '"') { ++pos; continue; }

            std::string key = parseString(content, pos);
            skipWs(content, pos);
            if (pos < content.size() && content[pos] == ':') ++pos;
            obj[key] = parseValue(content, pos);
        }
        result.push_back(std::move(obj));
    }

    return result;
}

void writeArray(const std::string& filePath,
                const std::vector<JsonObject>& objects,
                const std::vector<FieldDef>& fieldDefs) {
    size_t sep = filePath.find_last_of("/\\");
    if (sep != std::string::npos)
        std::filesystem::create_directories(filePath.substr(0, sep));

    // atomic write: 임시 파일에 먼저 쓴 뒤 rename으로 교체
    // → 쓰기 실패 시 기존 파일 보존
    std::string tmpPath = filePath + ".tmp";
    {
        std::ofstream file(tmpPath, std::ios::out | std::ios::trunc);
        if (!file.is_open())
            throw std::runtime_error("Cannot open temp file: " + tmpPath);

        file << "[\n";
        for (size_t i = 0; i < objects.size(); ++i) {
            const auto& obj = objects[i];
            file << "  {\n";
            for (size_t j = 0; j < fieldDefs.size(); ++j) {
                const auto& fd = fieldDefs[j];
                auto it = obj.find(fd.name);
                const std::string& val = (it != obj.end()) ? it->second : "";

                file << "    \"" << fd.name << "\": ";
                if (fd.type == ValueType::String)
                    file << '"' << escapeString(val) << '"';
                else
                    file << (val.empty() ? "0" : val);

                if (j + 1 < fieldDefs.size()) file << ',';
                file << '\n';
            }
            file << "  }";
            if (i + 1 < objects.size()) file << ',';
            file << '\n';
        }
        file << "]\n";
        file.close();
        if (!file)
            throw std::runtime_error("Write error on temp file: " + tmpPath);
    }
    std::filesystem::rename(tmpPath, filePath);
}

} // namespace JsonUtil
