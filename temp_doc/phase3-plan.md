# Phase 3 구현 계획 — UI 완성 + 주문 접수/승인/거절 PRD 준수

**브랜치**: `phase/3-order`  
**목표**: Phase 2에서 스켈레톤 수준으로 구현된 UI를 PRD 명세에 맞게 완성한다.  
ConsoleColor 유틸을 추가하고, 입력 유효성 검사·화면 레이아웃·FIFO 재고 할당 등 미구현 요구사항을 완성한다.

---

## Phase 2 대비 미구현 항목 (PRD 기준)

| 구분 | 미구현 항목 |
|------|------------|
| **공통** | `util/ConsoleColor.h` 없음, ANSI 컬러 미적용 |
| **메인 화면** | ASCII Art 로고, 시스템 현황(시료 수/총 재고/주문/생산라인), 2열 메뉴 레이아웃 |
| **시료 관리** | 중복 ID 검사, 수율 범위 검증(재입력 루프), 목록 페이지네이션(5행/페이지) |
| **주문 접수** | Y/N 확인 화면, 잘못된 입력 재입력 루프, 당일 기준 순번(현재는 전체 max) |
| **주문 승인/거절** | 번호 선택 방식(현재는 주문번호 직접 입력), 재고 확인 화면, 실생산량·총생산시간 표시, FIFO 가용 재고 계산 |
| **출고 처리** | 번호 선택 방식(현재는 주문번호 직접 입력) |

---

## 구현 대상 파일

```
util/
  ConsoleColor.h                  ← 신규: ANSI 컬러 유틸 (헤더 전용)

view/
  MenuView.h / MenuView.cpp       ← 수정: ASCII art, 시스템 현황, 2열 레이아웃, 컬러
  SampleView.h / SampleView.cpp   ← 수정: 페이지네이션, 컬러, 재고 0 강조
  OrderView.h / OrderView.cpp     ← 수정: Y/N 확인, 번호 선택, 재고확인 화면, 컬러

controller/
  SampleController.h / .cpp       ← 수정: 중복 ID 루프, 수율 검증 루프, 대소문자 무시 검색
  OrderController.h / .cpp        ← 수정: 당일 순번, 재입력 루프, 번호 선택, FIFO 가용 재고
  ReleaseController.h / .cpp      ← 수정: 번호 선택 방식

main.cpp                          ← 수정: ANSI 활성화(SetConsoleMode), 시스템 현황 집계 후 MenuView 전달

test/
  SampleControllerTest.cpp        ← 수정: 신규 유효성 검사 테스트 추가
  OrderControllerTest.cpp         ← 수정: FIFO 할당 테스트, 번호 선택 테스트 추가
  ReleaseControllerTest.cpp       ← 수정: 번호 선택 테스트 반영
```

---

## Step별 구현 순서 및 커밋 계획

---

### Step 0 — ConsoleColor 유틸 + main.cpp ANSI 활성화

**목표**: 모든 View에서 사용할 컬러 유틸 준비 및 Windows ANSI 처리 활성화

**파일**: `util/ConsoleColor.h`, `main.cpp`

#### ConsoleColor.h (헤더 전용)

```cpp
#pragma once
#include <ostream>
#include <string>

namespace ConsoleColor {

constexpr const char* RESET   = "\033[0m";
constexpr const char* WHITE   = "\033[97m";
constexpr const char* CYAN    = "\033[96m";    // 메뉴 번호 [N]
constexpr const char* BLUE    = "\033[94m";    // 테이블 헤더, RESERVED 배지
constexpr const char* YELLOW  = "\033[93m";    // 강조 수치, 경고 ※, PRODUCING 배지
constexpr const char* GREEN   = "\033[92m";    // 완료 메시지, CONFIRMED 배지
constexpr const char* RED     = "\033[91m";    // 오류, 재고 0 강조, 고갈
constexpr const char* PURPLE  = "\033[95m";    // RELEASE 배지
constexpr const char* ORANGE  = "\033[33m";    // PRODUCING 배지 (주황)

inline void print(std::ostream& out, const std::string& text, const char* color) {
    out << color << text << RESET;
}

inline void println(std::ostream& out, const std::string& text, const char* color) {
    out << color << text << RESET << "\n";
}

}  // namespace ConsoleColor
```

**main.cpp 수정** — ANSI 활성화 추가:
```cpp
#include <windows.h>

static void enableAnsiColor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    enableAnsiColor();
    ...
}
```

**커밋**:
```
feat: ConsoleColor 유틸 추가 및 main.cpp ANSI 컬러 활성화
```

---

### Step 1 — 메인 화면 개선 (MenuView)

**목표**: PRD UI 명세에 맞는 메인 화면 구현

**파일**: `view/MenuView.h`, `view/MenuView.cpp`, `main.cpp`

#### 메인 화면 구성 요소

**ASCII Art 로고** (하드코딩):
```
  ██████╗       ██████╗       ███████╗    ███╗
  ██╔════╝      ██╔════╝      ██╔════╝    ████║
  ███████╗      █████╗        █████╗      ██╔██║
  ╚════██║      ██╔══╝        ██╔══╝      ██ ██║
  ███████║      ███████╗      ███████╗    ██████║
  ╚══════╝      ╚══════╝      ╚══════╝    ╚═════╝
```

**시스템 현황 DTO**:
```cpp
struct SystemStatus {
    int sampleCount;
    int totalStock;
    int orderCount;      // REJECTED 제외
    int producingCount;
};
```

**시스템 현황 집계** (`main.cpp`의 `buildSystemStatus()`):
```cpp
static SystemStatus buildSystemStatus(IRepository<Sample>& sr, IOrderRepository& or_) {
    auto samples = sr.findAll();
    auto orders  = or_.findAll();
    int stock = 0;
    for (auto& s : samples) stock += s.stock;
    int orderCnt = 0, prodCnt = 0;
    for (auto& o : orders) {
        if (o.getStatus() == OrderStatus::REJECTED) continue;
        ++orderCnt;
        if (o.getStatus() == OrderStatus::PRODUCING) ++prodCnt;
    }
    return { (int)samples.size(), stock, orderCnt, prodCnt };
}
```

**MenuView 인터페이스 변경**:
```cpp
static void printMainMenu(const SystemStatus& status, std::ostream& out);
//  ↑ 시스템 현황 파라미터 추가
```

**2열 레이아웃**:
```
  [1] 시료 관리                          [2] 시료 주문
  [3] 주문 승인/거절                     [4] 모니터링
  [5] 생산라인 조회                      [6] 출고 처리
  [0] 종료
```

**커밋**:
```
feat: MenuView 개선 — ASCII art 로고, 시스템 현황, 2열 레이아웃, 컬러 적용
```

---

### Step 2 — 시료 관리 개선 (SampleView + SampleController, TDD)

**목표**: 입력 유효성 검사, 페이지네이션, 컬러 적용

**파일**: `view/SampleView.h/.cpp`, `controller/SampleController.h/.cpp`, `test/SampleControllerTest.cpp`

#### 유효성 검사 루프 (SampleController)

```cpp
// 중복 ID 검사 루프
while (true) {
    std::string id = SampleView::inputSampleId(in_, out_);
    if (sampleRepo_.findById(id)) {
        SampleView::printDuplicateId(out_);
        continue;
    }
    break;
}

// 수율 범위 검증 루프
while (true) {
    double yield = SampleView::inputYield(in_, out_);
    if (yield <= 0.0 || yield > 1.0) {
        SampleView::printInvalidYield(out_);
        continue;
    }
    break;
}
```

#### 페이지네이션 (SampleView)

```cpp
// 5행/페이지, [N] 다음, [B] 이전 (또는 [0] 위로)
static void printListPaged(const std::vector<Sample>& samples,
                           std::istream& in, std::ostream& out);
```

구현 로직:
```
const int PAGE_SIZE = 5;
int page = 0;
while (true) {
    int start = page * PAGE_SIZE;
    int end = min(start + PAGE_SIZE, (int)samples.size());
    // start ~ end 출력
    // 다음 페이지 있으면: [N] 다음  [0] 위로
    // 없으면: [0] 위로
    // 재고 0인 항목 → RED 색상
}
```

#### 테스트 추가 (SampleControllerTest)
- `RegisterSample_DuplicateId_RepromptsAndSucceeds` — 중복 입력 후 정상 등록
- `RegisterSample_InvalidYield_RepromptsAndSucceeds` — 수율 오류 후 정상 등록
- `SearchByName_CaseInsensitive_FindsMatch` — 대소문자 무시 검색

**커밋 순서**:
```
test: SampleController 유효성 검사·페이지네이션 테스트 추가
feat: SampleController + SampleView 유효성 검사, 페이지네이션, 컬러 적용
```

---

### Step 3 — 주문 접수 개선 (OrderView + OrderController, TDD)

**목표**: Y/N 확인 화면, 재입력 루프, 당일 순번, 컬러 적용

**파일**: `view/OrderView.h/.cpp`, `controller/OrderController.h/.cpp`, `test/OrderControllerTest.cpp`

#### Y/N 확인 화면

PRD 명세:
```
입력 내용 확인

시료        SiC 파워기판-6인치   (S-003)
고객        삼성전자 파운드리
수량        200 ea

[Y] 예약 접수    [N] 취소
선택 > Y
```

OrderController 흐름 변경:
```cpp
void OrderController::placeOrder() {
    // 1. 시료 ID 입력 (재입력 루프 포함)
    Sample sample = inputSampleWithRetry();
    // 2. 고객명 입력 (빈 값 재입력)
    std::string customer = inputCustomerWithRetry();
    // 3. 수량 입력 (1 미만 재입력)
    int qty = inputQuantityWithRetry();
    // 4. 확인 화면 출력
    OrderView::printOrderConfirm(sample, customer, qty, out_);
    // 5. Y/N 선택
    if (!OrderView::askConfirm(in_, out_)) return;  // N: 취소
    // 6. 저장
    ...
}
```

#### 당일 기준 순번 생성

```cpp
static std::string generateOrderNo(IOrderRepository& repo) {
    // 현재 날짜 YYYYMMDD 추출
    time_t now = time(nullptr);
    char dateBuf[9];
    strftime(dateBuf, sizeof(dateBuf), "%Y%m%d", localtime(&now));
    std::string prefix = std::string("ORD-") + dateBuf + "-";

    int count = 0;
    for (const auto& o : repo.findAll()) {
        if (o.orderNo.rfind(prefix, 0) == 0) ++count;
    }
    char seqBuf[5];
    snprintf(seqBuf, sizeof(seqBuf), "%04d", count + 1);
    return prefix + seqBuf;
}
```

#### 테스트 추가 (OrderControllerTest)
- `PlaceOrder_Confirms_SavesReserved` — Y 선택 시 저장
- `PlaceOrder_Cancels_NoSave` — N 선택 시 저장 안 함
- `PlaceOrder_InvalidSampleId_Reprompts` — 재입력 루프 후 성공
- `PlaceOrder_DayBasedSequence` — 당일 기준 순번

**커밋 순서**:
```
test: OrderController 주문 접수 개선 테스트 추가
feat: OrderController + OrderView 주문 접수 개선 — Y/N 확인, 재입력 루프, 당일 순번, 컬러
```

---

### Step 4 — 주문 승인/거절 개선 (OrderController, TDD)

**목표**: 번호 선택, 재고 확인 화면, 실생산량 표시, FIFO 가용 재고 계산

**파일**: `controller/OrderController.h/.cpp`, `view/OrderView.h/.cpp`, `test/OrderControllerTest.cpp`

#### 번호 선택 방식

UI 변경:
```
번호    주문번호       고객              시료                    수량      상태
[1]     ORD-0041  ...
[2]     ORD-0042  ...

승인할 번호 > 2
```

OrderController:
```cpp
void OrderController::processApproval() {
    auto reserved = orderRepo_.findByStatus(OrderStatus::RESERVED);
    if (reserved.empty()) { OrderView::printNoReservedOrders(out_); return; }

    OrderView::printOrderList(reserved, out_);
    int idx = OrderView::selectOrderIndex(reserved.size(), in_, out_);
    Order& order = reserved[idx - 1];
    ...
}
```

#### FIFO 가용 재고 계산

```cpp
static int calcAvailableStock(const Sample& sample,
                               const std::vector<Order>& allOrders) {
    // 가용 재고 = 현재 재고 - CONFIRMED 상태 동일 시료 미출고 수량 합산
    int reserved = 0;
    for (const auto& o : allOrders) {
        if (o.sampleId == sample.id && o.getStatus() == OrderStatus::CONFIRMED)
            reserved += o.quantity;
    }
    return sample.stock - reserved;
}
```

#### 재고 확인 화면

PRD 명세:
```
재고 확인 중...

시료          SiC 파워기판-6인치      현재 재고   30 ea
주문 수량     200 ea                  부족분      170 ea

재고 부족. 부족분 170 ea (실생산량 206 ea / 총 3,060 min)
[Y] 승인    [N] 주문 거절
```

OrderView 추가:
```cpp
static void printStockCheckResult(const Sample& s, int qty, int available,
                                   std::ostream& out);
static void printProductionInfo(int shortage, int prodQty, int totalMin,
                                 std::ostream& out);
```

#### 테스트 추가 (OrderControllerTest)
- `ProcessApproval_SelectByNumber_CorrectOrder` — 번호로 정확한 주문 선택
- `ProcessApproval_FifoStock_ConfirmedAlreadyReserved` — FIFO 가용 재고 반영
- `ProcessApproval_ShowsStockScreen` — 재고 확인 화면 출력

**커밋 순서**:
```
test: OrderController 주문 승인 개선 테스트 추가 (번호 선택, FIFO 재고, 재고확인 화면)
feat: OrderController + OrderView 주문 승인/거절 개선 — 번호 선택, FIFO 재고, 화면 개선, 컬러
```

---

### Step 5 — 출고 처리 개선 (ReleaseController, TDD)

**목표**: 번호 선택 방식으로 변경, 결과 화면 개선

**파일**: `controller/ReleaseController.h/.cpp`, `view/OrderView.h/.cpp`, `test/ReleaseControllerTest.cpp`

PRD 명세:
```
번호    주문번호       고객          시료                    수량
[1]     ORD-0042  SK하이닉스    실리콘 웨이퍼-8인치     150 ea

출고할 번호 > 1
```

결과 화면:
```
출고 처리 완료.

주문번호     ORD-20260416-0042
출고수량     150 ea
처리일시     2026-04-16 09:34:02
상태         CONFIRMED  →  [ RELEASE ]
```

#### 테스트 추가 (ReleaseControllerTest)
- `ProcessRelease_SelectByNumber_ReleasesCorrectOrder` — 번호 선택
- `ProcessRelease_ShowsResultScreen` — 결과 화면 (주문번호, 수량, 처리일시)

**커밋 순서**:
```
test: ReleaseController 번호 선택 방식 테스트 추가
feat: ReleaseController + OrderView 출고 처리 개선 — 번호 선택, 결과 화면, 컬러
```

---

### Step 6 — vcxproj 업데이트

**목표**: 신규 파일(`util/ConsoleColor.h`)을 vcxproj에 등록

```xml
<ClInclude Include="util\ConsoleColor.h" />
```

**커밋**:
```
chore: ConsoleColor.h vcxproj 등록
```

---

## 의존 방향 (엄격 준수)

```
View → Controller → Repository → Model
```

- `ConsoleColor` namespace는 `util/ConsoleColor.h` (헤더 전용)
- View에서만 `ConsoleColor::print/println` 호출 (Controller·Model 직접 출력 금지)
- `SystemStatus` struct를 `model/` 또는 별도 헤더로 분리 고려 (main.cpp와 MenuView 공유)

---

## 커밋 순서 요약

| 순서 | 단계 | 커밋 메시지 |
|------|------|------------|
| 1 | Step 0 | `feat: ConsoleColor 유틸 추가 및 main.cpp ANSI 컬러 활성화` |
| 2 | Step 1 | `feat: MenuView 개선 — ASCII art 로고, 시스템 현황, 2열 레이아웃, 컬러 적용` |
| 3 | Step 2 RED | `test: SampleController 유효성 검사·페이지네이션 테스트 추가` |
| 4 | Step 2 GREEN | `feat: SampleController + SampleView 유효성 검사, 페이지네이션, 컬러 적용` |
| 5 | Step 3 RED | `test: OrderController 주문 접수 개선 테스트 추가` |
| 6 | Step 3 GREEN | `feat: OrderController + OrderView 주문 접수 개선 — Y/N 확인, 재입력 루프, 당일 순번, 컬러` |
| 7 | Step 4 RED | `test: OrderController 주문 승인 개선 테스트 추가 (번호 선택, FIFO 재고, 재고확인 화면)` |
| 8 | Step 4 GREEN | `feat: OrderController + OrderView 주문 승인/거절 개선 — 번호 선택, FIFO 재고, 화면 개선, 컬러` |
| 9 | Step 5 RED | `test: ReleaseController 번호 선택 방식 테스트 추가` |
| 10 | Step 5 GREEN | `feat: ReleaseController + OrderView 출고 처리 개선 — 번호 선택, 결과 화면, 컬러` |
| 11 | Step 6 | `chore: ConsoleColor.h vcxproj 등록` |

---

## PR 정보

- **제목**: `[Phase 3] UI 완성 + 주문 접수/승인/거절 PRD 준수`
- **base**: `main`
- **compare**: `phase/3-order`
