# Phase 5 구현 계획 — 입력 유효성 완성 + 생산완료 자동처리 + 통합 테스트

## Context

Phase 1~4로 도메인·Repository·Controller·View·UX가 완성되었다.
Phase 5는 세 가지 미완성 항목을 마무리하여 PRD를 100% 충족한다.

---

## 미완성 항목

### 1. placeOrder 입력 유효성 검사 미완성
- 고객명 빈 문자열 → 현재 그대로 진행 (재프롬프트 없음)
- 수량 0 이하 → 현재 그대로 진행 (재프롬프트 없음)

### 2. 생산 완료 자동 처리 미완성
- `processCompletedJobs()`는 `private`이고 메뉴 [5] 생산라인 조회 시에만 호출됨
- 메뉴 [3] 승인 / [6] 출고 / [4] 모니터링 시에도 완료된 잡이 CONFIRMED 전환되지 않음
- 메인 루프에서 매 턴 자동 처리해야 PRD 요건 충족

### 3. 통합 테스트 부재
- 전체 주문 흐름(RESERVED → CONFIRMED → RELEASE)을 검증하는 자동화 테스트 없음
- `ProductionLine` 재시작 복원 로직 테스트 없음

---

## 구현 순서 (TDD 준수)

### Step 1 — placeOrder 유효성 강화 (TDD)

**신규 View 메서드 (`view/OrderView.h/.cpp`):**
```cpp
static void printInvalidCustomerName(std::ostream& out);
static void printInvalidQuantity(std::ostream& out);
```

**Controller 수정 (`controller/OrderController.cpp`):**
```cpp
// 고객명 빈값 재프롬프트
std::string customer;
while (true) {
    customer = OrderView::inputCustomerName(in_, out_);
    if (!customer.empty()) break;
    OrderView::printInvalidCustomerName(out_);
}

// 수량 양수 재프롬프트
int qty;
while (true) {
    qty = OrderView::inputQuantity(in_, out_);
    if (qty > 0) break;
    OrderView::printInvalidQuantity(out_);
}
```

**테스트 (`test/OrderControllerTest.cpp` 추가):**
```
PlaceOrder_EmptyCustomerName_Reprompts
PlaceOrder_ZeroQuantity_Reprompts
```

**커밋:** `test: placeOrder 고객명·수량 유효성 테스트` → `feat: placeOrder 고객명·수량 재프롬프트 추가`

---

### Step 2 — 생산 완료 자동 처리 (TDD)

**Controller 수정 (`controller/ProductionController.h`):**
```cpp
void processCompletedJobs();  // private → public
```

**main.cpp 수정:**
```cpp
// 메인 루프 상단에서 매 턴 생산 완료 체크
while (true) {
    prodCtrl.processCompletedJobs();  // 완료된 잡 자동 CONFIRMED 전환
    SystemStatus status = buildSystemStatus(sampleRepo, orderRepo);
    MenuView::printMainMenu(status, std::cout);
    ...
}
```

**테스트 (`test/ProductionControllerTest.cpp` 추가):**
```
ProcessCompletedJobs_CompletedJob_SetsConfirmedAndUpdatesStock
ProcessCompletedJobs_InProgressJob_NoUpdate
ProcessCompletedJobs_MultipleCompleted_ProcessesAll
```

**커밋:** `test: processCompletedJobs public 접근 테스트` → `feat: processCompletedJobs public 전환 + main 루프 자동 호출`

---

### Step 3 — 통합 테스트 (`test/IntegrationTest.cpp` 신규)

**테스트 항목:**
```
Integration_FullOrderFlow_ReservedToRelease
  — 시료 등록 → 주문 접수 → 승인(재고 충분) → 출고 전체 흐름
  — Mock 없이 In-memory 데이터 구조 사용
  — 상태 전환 순서 검증: RESERVED → CONFIRMED → RELEASE

Integration_InsufficientStock_ProducingFlow
  — 재고 부족 승인 → PRODUCING → 생산완료(processCompletedJobs) → CONFIRMED

Integration_ProductionLineRestore_FromProducingOrders
  — PRODUCING 주문을 직접 설정 후 buildProductionLine 로직 검증
  — 재시작 시 라인 복원 순서(startTime 정렬) 확인
```

**커밋:** `test: 통합 시나리오 테스트 (E2E 흐름 검증)`

---

### Step 4 — vcxproj 등록

신규 파일:
- `test/IntegrationTest.cpp`

**커밋:** `chore: Phase 5 신규 파일 vcxproj 등록`

---

## 수정/추가 파일 목록

| 구분 | 파일 |
|------|------|
| **수정** | `controller/OrderController.cpp` |
| **수정** | `controller/ProductionController.h` |
| **수정** | `view/OrderView.h`, `view/OrderView.cpp` |
| **수정** | `main.cpp` |
| **수정** | `test/OrderControllerTest.cpp` |
| **수정** | `test/ProductionControllerTest.cpp` |
| **신규 테스트** | `test/IntegrationTest.cpp` |
| **수정** | `SampleOrderSystem.vcxproj` |

---

## 검증

- Debug|x64 빌드 성공, 모든 테스트 PASSED (기존 94개 + 신규 ~8개)
- [2] 시료 주문 → 빈 고객명·0 수량 입력 시 재프롬프트 확인
- [5] 생산라인 조회 없이도 완료된 잡이 CONFIRMED 전환되는지 확인
- 전체 흐름 수동 E2E: 시료 등록 → 주문 → 승인 → 출고 확인
