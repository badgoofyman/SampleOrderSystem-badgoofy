# F-003 주문 승인/거절 (Order Approval & Rejection)

## 개요
생산담당자가 RESERVED 상태 주문을 확인하고 승인 또는 거절하는 기능.  
승인 시 현재 재고를 **접수 순서(FIFO)** 로 우선 할당한다.  
메인 메뉴 **[3] 주문 승인/거절** 에 해당한다.

---

## 관련 파일

| 레이어 | 파일 |
|--------|------|
| Model | `model/Order.h`, `model/Sample.h` |
| Repository | `repository/OrderRepository.h`, `repository/SampleRepository.h` |
| Controller | `controller/OrderController.h`, `controller/OrderController.cpp` |
| View | `view/OrderView.h`, `view/OrderView.cpp` |
| Util | `util/ProductionCalculator.h` |
| DB | `data/orders.json`, `data/samples.json` |

---

## 기능 상세

### F-003-1 RESERVED 목록 표시

- `OrderRepository::findByStatus(RESERVED)` 로 조회
- 주문번호 / 시료 ID / 시료명 / 고객명 / 수량 테이블 출력
- 목록 없을 시 안내 메시지 출력 후 메인 메뉴로 복귀

### F-003-2 승인 처리

**재고 선순위 할당 (FIFO)**

동일 시료에 대해 복수의 RESERVED 주문이 존재하면, 주문번호(접수 순) 기준으로 재고를 먼저 배정한다.

```
가용 재고 = 현재 재고 - 이미 CONFIRMED 상태인 동일 시료의 미출고 수량
```

> PRODUCING 중인 주문은 아직 재고 차감 전이므로 가용 재고 계산에서 제외한다.

**승인 분기**

```
가용 재고 ≥ 주문 수량
  → 재고 차감 (stock -= quantity)
  → 상태: RESERVED → CONFIRMED
  → samples.json, orders.json 저장

가용 재고 < 주문 수량
  → 부족분 = 주문 수량 - 가용 재고
  → 실 생산량 계산 (ProductionCalculator 호출)
  → productionStartTime = 현재 UTC epoch (time(nullptr))
  → 상태: RESERVED → PRODUCING
  → orders.json 저장 (productionStartTime 포함)
```

### F-003-3 거절 처리

- 상태: RESERVED → REJECTED
- 재고 변경 없음
- orders.json 저장

---

## 재고 차감 시점 규칙

| 전환 | 재고 차감 |
|------|----------|
| RESERVED → CONFIRMED | **즉시 차감** |
| RESERVED → PRODUCING | 차감 없음 (생산 완료 후 CONFIRMED 전환 시 차감) |
| PRODUCING → CONFIRMED | **즉시 차감** (F-005 에서 자동 처리) |

---

## 예외 처리

| 상황 | 처리 |
|------|------|
| RESERVED 주문 없음 | 안내 메시지 후 메뉴 복귀 |
| 잘못된 주문번호 입력 | 오류 메시지(빨강) 후 재입력 |
| JSON 쓰기 실패 | 오류 메시지, 상태 롤백 |

---

## 테스트 케이스

| ID | 시나리오 | 기대 결과 |
|----|----------|----------|
| TC-003-01 | 재고 충분 → 승인 | CONFIRMED 전환, 재고 차감 |
| TC-003-02 | 재고 부족 → 승인 | PRODUCING 전환, productionStartTime 기록, 재고 유지 |
| TC-003-03 | 재고 0 → 승인 | PRODUCING 전환 (부족분 = 전체 수량) |
| TC-003-04 | 거절 | REJECTED 전환, 재고 불변 |
| TC-003-05 | FIFO 재고 할당: 재고 15, 주문A 10(선) + 주문B 10(후) → A 승인 | A: CONFIRMED(재고 10 차감), B: PRODUCING |
| TC-003-06 | FIFO 재고 할당: 재고 20, 주문A 10(선) + 주문B 10(후) → A 승인 | A: CONFIRMED, 잔여 재고 10으로 B도 CONFIRMED 가능 |
| TC-003-07 | RESERVED 목록 없음 | 안내 메시지 출력 |
