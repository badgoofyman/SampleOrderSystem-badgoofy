# F-004 모니터링 (Monitoring)

## 개요
주문 상태별 건수 집계와 시료별 재고 현황을 한 화면에 표시하는 기능.  
메인 메뉴 **[4] 모니터링** 에 해당한다.

---

## 관련 파일

| 레이어 | 파일 |
|--------|------|
| Repository | `repository/OrderRepository.h`, `repository/SampleRepository.h` |
| Controller | `controller/SampleController.h`, `controller/OrderController.h` |
| View | `view/MonitorView.h`, `view/MonitorView.cpp` |
| DB | `data/orders.json`, `data/samples.json` |

---

## 기능 상세

### F-004-1 주문 상태별 집계

`OrderRepository::findAll()` 로 전체 주문을 로드한 뒤 상태별 카운트를 산출한다.

| 표시 항목 | 집계 대상 |
|----------|----------|
| RESERVED | status == RESERVED |
| PRODUCING | status == PRODUCING |
| CONFIRMED | status == CONFIRMED |
| RELEASE | status == RELEASE |

> **REJECTED는 집계에서 제외한다.**

**출력 예시**
```
=== 주문 현황 ===
  RESERVED  :  3건
  PRODUCING :  1건
  CONFIRMED :  2건
  RELEASE   :  5건
```

### F-004-2 시료별 재고 현황

모든 시료에 대해 아래 순서로 재고 상태를 판정한다.

```
활성 주문 수량 합계 = 해당 시료의 PRODUCING 주문 수량 합 + CONFIRMED 주문 수량 합

재고 상태 판정:
  stock == 0                          → 고갈  (빨강)
  stock > 0 && stock < 활성 주문 합계  → 부족  (노랑)
  stock > 0 && stock >= 활성 주문 합계 → 여유  (초록)
```

**출력 예시**
```
=== 시료별 재고 현황 ===
  S-001  실리콘 웨이퍼-8인치   재고:  50  [여유]
  S-002  GaAs 기판-4인치       재고:   5  [부족]
  S-003  SiC 웨이퍼-6인치      재고:   0  [고갈]
```

---

## 예외 처리

| 상황 | 처리 |
|------|------|
| 주문 데이터 없음 | 전체 상태 0건으로 표시 |
| 시료 데이터 없음 | 재고 현황 없음 메시지 출력 |

---

## 테스트 케이스

| ID | 시나리오 | 기대 결과 |
|----|----------|----------|
| TC-004-01 | REJECTED 주문 포함 시 집계 | REJECTED 제외한 건수만 표시 |
| TC-004-02 | 재고 0인 시료 | 고갈(빨강) 표시 |
| TC-004-03 | 재고 > 0, 활성 주문 합 초과 | 부족(노랑) 표시 |
| TC-004-04 | 재고 > 0, 활성 주문 합 이하 | 여유(초록) 표시 |
| TC-004-05 | 활성 주문 없는 시료, 재고 > 0 | 여유(초록) 표시 |
| TC-004-06 | 주문·시료 모두 없음 | 0건 집계, 재고 없음 메시지 |
