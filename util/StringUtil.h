#pragma once
#include <string>

namespace StringUtil {

// UTF-8 문자열의 터미널 표시 너비 계산
// ASCII: 1칸, 2바이트 UTF-8: 1칸, 3바이트 UTF-8(한글 등): 2칸, 4바이트: 2칸
inline int displayWidth(const std::string& s) {
    int width = 0;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c < 0x80) {
            width += 1; i += 1;
        } else if (c < 0xE0) {
            width += 1; i += 2;
        } else if (c < 0xF0) {
            width += 2; i += 3;
        } else {
            width += 2; i += 4;
        }
    }
    return width;
}

// std::setw 보정값: 원하는 표시 너비(칸)에 맞는 setw 인자 반환
// 한글이 포함된 경우 setw(n)만으로는 칸이 부족하므로 바이트 차이만큼 보정
inline int setWidth(const std::string& s, int targetDisplayWidth) {
    int dw = displayWidth(s);
    int bw = static_cast<int>(s.size());
    return targetDisplayWidth + (bw - dw);
}

} // namespace StringUtil
