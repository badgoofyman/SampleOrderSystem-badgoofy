# Phase 5 검토 체크리스트

**브랜치**: `phase/5-final`  
**검토 시점**: PR 리뷰 전 로컬에서 직접 확인

---

## 1. 빌드 및 테스트

- [ ] Debug|x64 빌드 성공 — 오류·경고 0건
- [ ] Release|x64 빌드 성공
- [ ] 모든 테스트 PASSED — 기존 94개 포함 신규 테스트 전체 통과
- [ ] 실패 테스트 0건

---

## 2. placeOrder 유효성 강화

### 2-1. 고객명 유효성
- [ ] 빈 문자열 입력 시 오류 메시지 출력
- [ ] 빈 문자열 입력 시 재프롬프트 루프 동작
- [ ] 정상 입력 시 루프 종료
- [ ] `OrderView::printInvalidCustomerName` 메서드 존재

### 2-2. 수량 유효성
- [ ] 0 이하 입력 시 오류 메시지 출력
- [ ] 0 이하 입력 시 재프롬프트 루프 동작
- [ ] 양수 입력 시 루프 종료
- [ ] `OrderView::printInvalidQuantity` 메서드 존재

### 2-3. 테스트
- [ ] `PlaceOrder_EmptyCustomerName_Reprompts` — save 미호출, 오류 메시지 포함
- [ ] `PlaceOrder_ZeroQuantity_Reprompts` — save 미호출, 오류 메시지 포함

---

## 3. 생산 완료 자동 처리

### 3-1. ProductionController
- [ ] `processCompletedJobs()` 접근 제어자 `public`으로 변경
- [ ] 외부에서 직접 호출 가능 확인

### 3-2. main.cpp
- [ ] 메인 루프 상단에서 `prodCtrl.processCompletedJobs()` 매 턴 호출
- [ ] 호출 위치: `MenuView::printMainMenu` 이전

### 3-3. 테스트
- [ ] `ProcessCompletedJobs_CompletedJob_SetsConfirmedAndUpdatesStock` — public 접근
- [ ] `ProcessCompletedJobs_InProgressJob_NoUpdate` — 미완료 잡은 변화 없음
- [ ] `ProcessCompletedJobs_MultipleCompleted_ProcessesAll` — 여러 완료 잡 연속 처리

---

## 4. 통합 테스트 (`IntegrationTest.cpp`)

- [ ] `Integration_FullOrderFlow_ReservedToRelease`
  - 시료 등록 → 주문 접수(RESERVED) → 재고 충분 승인(CONFIRMED) → 출고(RELEASE) 흐름
  - 각 단계 상태 검증
  - Mock 없이 실제 데이터 객체 직접 조작

- [ ] `Integration_InsufficientStock_ProducingFlow`
  - 재고 부족 승인 → PRODUCING + ProductionLine enqueue
  - processCompletedJobs 호출 후 CONFIRMED 전환
  - 재고 stock += (productionQty - quantity) 검증

- [ ] `Integration_ProductionLineRestore_SortedByStartTime`
  - PRODUCING 주문 2개 (startTime 순서 다름)
  - buildProductionLine 로직 검증: 빠른 startTime이 first
  - 큐 size 및 first 항목 orderNo 검증

---

## 5. 수동 시나리오

### 시나리오 A — 고객명 빈값 재프롬프트
1. [2] 시료 주문 → 시료 ID 입력 → 고객명에 빈 Enter
2. 오류 메시지 출력 + 재프롬프트 확인
3. 정상 고객명 입력 후 진행 확인

### 시나리오 B — 수량 0 재프롬프트
1. [2] 시료 주문 → 시료 ID → 고객명 → 수량에 `0` 입력
2. 오류 메시지 출력 + 재프롬프트 확인

### 시나리오 C — 생산 완료 자동 처리
1. 재고 부족 주문 승인 → PRODUCING
2. JSON에서 `productionStartTime`을 과거로 수동 수정
3. [1] 시료 관리 등 다른 메뉴 진입
4. 메인 메뉴로 복귀 시 자동 CONFIRMED 전환 메시지 확인

---

## 6. Git 이력 검토

- [ ] TDD 순서 준수 — `test:` 커밋 후 `feat:` 커밋
- [ ] Step별 커밋 분리
- [ ] 불필요한 임시 커밋 없음

---

## 7. PR 발행 전 최종 확인

- [ ] 위 1~6번 항목 전체 통과
- [ ] PR 본문에 테스트 결과 포함
- [ ] `phase/5-final`이 최신 `main`과 충돌 없음
