# F-010 콘솔 컬러 출력 (Console Color)

## 개요
콘솔 화면에서 상태·경고·오류를 색상으로 구분하여 가독성을 높이는 유틸리티.  
View 레이어에서만 호출하며, Windows ANSI 이스케이프 코드를 사용한다.

---

## 관련 파일

| 레이어 | 파일 |
|--------|------|
| Util | `util/ConsoleColor.h` |

---

## 컬러 표준

| 색상 | ANSI 코드 | 용도 |
|------|----------|------|
| 기본 (흰색) | `\033[0m` | 일반 텍스트, 메뉴 항목, 입력 프롬프트 |
| 초록 | `\033[32m` | 성공, 정상 상태 (등록 완료, CONFIRMED, RELEASE, 재고 여유) |
| 노랑 | `\033[33m` | 경고, 주의 상태 (RESERVED, PRODUCING, 재고 부족) |
| 빨강 | `\033[31m` | 오류, 위험 상태 (입력 오류, REJECTED, 재고 고갈) |
| 시안 | `\033[36m` | 강조, 제목 (메뉴 헤더, 섹션 구분선) |
| 자홍 | `\033[35m` | 정보 강조 (주문번호, 시료 ID) |

---

## 인터페이스

```cpp
namespace ConsoleColor {

    void print(const std::string& text, const std::string& colorCode);

    void green  (const std::string& text);
    void yellow (const std::string& text);
    void red    (const std::string& text);
    void cyan   (const std::string& text);
    void magenta(const std::string& text);
    void reset  ();

    // 줄바꿈 포함 버전
    void greenln  (const std::string& text);
    void yellowln (const std::string& text);
    void redln    (const std::string& text);
    void cyanln   (const std::string& text);
    void magentln (const std::string& text);

} // namespace ConsoleColor
```

---

## 사용 예시

```cpp
// 메뉴 헤더
ConsoleColor::cyanln("=== S-Semi 시료 생산주문관리 시스템 ===");

// 성공 메시지
ConsoleColor::greenln("시료 등록이 완료되었습니다.");

// 오류 메시지
ConsoleColor::redln("오류: 중복된 시료 ID입니다.");

// 주문번호 강조
ConsoleColor::magenta("ORD-20260612-0001");
std::cout << " 주문이 접수되었습니다.\n";

// 재고 상태
ConsoleColor::greenln("[여유]");
ConsoleColor::yellowln("[부족]");
ConsoleColor::redln("[고갈]");
```

---

## 구현 원칙

- 컬러 리셋(`\033[0m`)은 각 함수 출력 직후 반드시 수행한다.
- View 레이어에서만 호출한다. Controller·Model·Repository에서 직접 출력 금지.
- Windows에서 ANSI 이스케이프 코드 활성화가 필요한 경우 `main.cpp` 시작 시 `SetConsoleMode`를 호출하여 `ENABLE_VIRTUAL_TERMINAL_PROCESSING` 플래그를 설정한다.

```cpp
// main.cpp — ANSI 컬러 활성화 (Windows)
#ifdef _WIN32
#include <windows.h>
void enableAnsiColor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#endif
```

---

## 주문 상태별 컬러 매핑

| 상태 | 컬러 |
|------|------|
| RESERVED | 노랑 |
| PRODUCING | 노랑 |
| CONFIRMED | 초록 |
| RELEASE | 초록 |
| REJECTED | 빨강 |

## 재고 상태별 컬러 매핑

| 표기 | 컬러 |
|------|------|
| [여유] | 초록 |
| [부족] | 노랑 |
| [고갈] | 빨강 |
