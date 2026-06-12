# Phase 4 구현 계획 — 모니터링·생산라인 UX 완성 + 통합 품질 마무리

## Context

Phase 1~3에서 도메인 모델·Repository·Controller·View 기초 및 UX 핵심(컬러·유효성·FIFO)을 완성했다.  
Phase 4는 아직 Plain-text 수준인 **모니터링(메뉴 4)**과 **생산라인 조회(메뉴 5)** 화면을 PRD 수준으로 완성하고,  
전체 시스템의 통합 품질을 마무리한다.

---

## 구현 범위

### 1. MonitorView UX 완성
- 상태별 배지 색상 (RESERVED=Blue, PRODUCING=Orange, CONFIRMED=Green, RELEASE=Purple)
- 재고 상태 색상화: **여유**=Green, **부족**=Yellow, **고갈**=Red
- 테이블 레이아웃 박스 스타일 (Phase 3의 SampleView와 동일한 ─┬┼─ 박스)

### 2. ProductionView UX 완성
- 현재 생산 중 항목: 잔여 시간 실시간 계산 및 표시
- 대기 큐 진입 예정 시각 표시
- 생산 완료 자동 전환 메시지 색상화

### 3. 테스트 강화
- `MonitorViewTest.cpp` — 재고 상태 색상 문자열 포함 여부 검증
- `ProductionControllerTest.cpp` — 다중 잡 FIFO 순서 검증, 완료 후 재고 반영 정확도

### 4. 코드 품질 마무리
- `SampleView::printListPaged` 재고 0 항목 Red 강조 (Phase 3 미완성 항목)
- `MonitorView::printStockStatus` 상태 문자열 컬러 일관성 확보

---

## 구현 순서 (TDD 준수)

### Step 1 — MonitorView 테스트 작성 → 구현

**테스트 항목 (`test/MonitorViewTest.cpp` 신규):**
```
PrintOrderSummary_ContainsStatusLabels
PrintStockStatus_ExcessStock_ContainsGreen
PrintStockStatus_InsufficientStock_ContainsYellow
PrintStockStatus_DepletedStock_ContainsRed
```

**구현 (`view/MonitorView.cpp`):**
```
- 상태 요약 박스 (테이블 스타일)
- RESERVED / PRODUCING / CONFIRMED / RELEASE 배지 컬러
- 재고 상태 테이블 컬러 (여유=Green, 부족=Yellow, 고갈=Red)
```

**커밋:** `test: MonitorView 재고 상태 컬러 테스트` → `feat: MonitorView UX 완성 — 박스 레이아웃 + 상태 컬러`

---

### Step 2 — ProductionView 테스트 작성 → 구현

**테스트 항목 (`test/ProductionViewTest.cpp` 신규):**
```
PrintProductionLine_ActiveJob_ShowsRemainingTime
PrintProductionLine_MultipleJobs_ShowsFifoOrder
PrintEmpty_ShowsMessage
PrintCompleted_ShowsOrderNo
```

**구현 (`view/ProductionView.cpp`):**
```
- 생산중 항목: 잔여 시간 (분) 계산 및 Yellow 강조
- 대기 항목: 예상 시작 시각 출력
- 생산완료 메시지 Green 색상
```

**커밋:** `test: ProductionView 단위 테스트` → `feat: ProductionView UX 완성 — 잔여시간 + 컬러`

---

### Step 3 — SampleView 재고 0 강조

**구현 (`view/SampleView.cpp` 수정):**
- `printListPaged` / `printList` 에서 `s.stock == 0` 인 행 → Red 컬러로 출력

**테스트:** `SampleViewTest.cpp` 신규
```
PrintList_ZeroStock_ContainsRedAnsi
```

**커밋:** `test: SampleView 재고 0 강조 테스트` → `feat: SampleView 재고 0 Red 강조`

---

### Step 4 — ProductionController 다중 잡 테스트 강화

**기존 테스트 파일 (`test/ProductionControllerTest.cpp`) 추가 케이스:**
```
ShowProductionLine_MultipleJobs_FifoOrderPreserved
  — 2개 잡 큐에 넣고 첫 번째만 완료 시 두 번째가 current가 되는지 확인
ShowProductionLine_CompletedJob_StockUpdatedCorrectly
  — 완료 후 sample.stock += (productionQty - quantity) 정확도 검증
```

**커밋:** `test: ProductionController 다중 잡 FIFO 및 재고 반영 테스트`

---

### Step 5 — vcxproj 등록 + 빌드 검증

새로 추가되는 파일:
- `test/MonitorViewTest.cpp`
- `test/ProductionViewTest.cpp`
- `test/SampleViewTest.cpp`

**커밋:** `chore: Phase 4 신규 테스트 파일 vcxproj 등록`

---

## 수정/추가 파일 목록

| 구분 | 파일 |
|------|------|
| **수정** | `view/MonitorView.cpp`, `view/ProductionView.cpp`, `view/SampleView.cpp` |
| **신규 테스트** | `test/MonitorViewTest.cpp`, `test/ProductionViewTest.cpp`, `test/SampleViewTest.cpp` |
| **수정 테스트** | `test/ProductionControllerTest.cpp` |
| **수정** | `SampleOrderSystem.vcxproj` |

---

## 검증

- Debug|x64 빌드 성공, 모든 테스트 PASSED (기존 77개 + 신규 ~10개)
- [4] 모니터링 화면 컬러 및 레이아웃 수동 확인
- [5] 생산라인 화면 잔여시간 출력 수동 확인
- [2]→[3]→[6] 전체 흐름 수동 E2E 확인
