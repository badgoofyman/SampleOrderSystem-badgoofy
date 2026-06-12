# F-005 생산라인 조회 (Production Line View)

## 개요
현재 생산 중인 항목과 대기 큐를 조회하고, 실제 경과 시간 기준으로 생산 완료 항목을 자동 처리하는 기능.  
메인 메뉴 **[5] 생산라인 조회** 에 해당한다.

---

## 관련 파일

| 레이어 | 파일 |
|--------|------|
| Model | `model/ProductionLine.h`, `model/ProductionLine.cpp` |
| Repository | `repository/OrderRepository.h`, `repository/SampleRepository.h` |
| Controller | `controller/ProductionController.h`, `controller/ProductionController.cpp` |
| View | `view/ProductionView.h`, `view/ProductionView.cpp` |
| Util | `util/ProductionCalculator.h` |
| DB | `data/orders.json`, `data/samples.json` |

---

## 기능 상세

### F-005-0 앱 시작 시 자동 완료 처리 (최우선)

앱 시작(`main.cpp`) 직후, 메인 화면 표시 전에 아래를 수행한다.

```
1. OrderRepository::findByStatus(PRODUCING) — 전체 생산 중 주문 조회
2. productionStartTime 오름차순(FIFO) 정렬
3. 각 주문에 대해:
     경과 시간(초) = time(nullptr) - productionStartTime
     총 생산시간(초) = avgProductionTime(min) × 실생산량 × 60
   if (경과 시간 >= 총 생산시간):
     stock += 실생산량            // SampleRepository 업데이트
     stock -= quantity            // CONFIRMED 전환 시 재고 차감
     order.status = CONFIRMED
     OrderRepository::update(order)
     SampleRepository::update(sample)
```

- 재실행 후 생산이 완료된 주문은 메인 화면 진입 전에 CONFIRMED로 전환된다.
- `productionStartTime`이 orders.json에 저장되므로 재실행 후에도 경과 시간을 정확히 계산한다.

### F-005-1 메뉴 진입 시 자동 완료 처리 재수행

`[5]` 메뉴 진입 시 F-005-0과 동일한 완료 처리를 재수행한다.  
(앱 시작 이후 메뉴 탐색 중 추가로 경과된 시간을 반영하기 위함)

```
1. OrderRepository::findByStatus(PRODUCING) 로 생산 중 주문 목록 조회
2. 각 주문에 대해 완료 조건 평가 → 충족 시 CONFIRMED 전환 + 재고 반영
3. 완료 처리된 주문 수를 화면에 안내
```

### F-005-2 FIFO 큐 구성

자동 완료 처리 후 남은 PRODUCING 주문을 `productionStartTime` 오름차순으로 정렬한다.

- **첫 번째 항목** = 현재 생산 중 (진행 중 표시)
- **나머지 항목** = 대기 큐 (순번 표시)

### F-005-3 화면 출력

**현재 생산 중**
```
=== 현재 생산 중 ===
  주문번호 : ORD-20260612-0001
  시료     : S-001 실리콘 웨이퍼-8인치
  생산 수량: 25 ea
  시작 시각: 2026-06-12 09:00:00
  경과 시간: 120 분
  남은 시간: 2880 분
```

**대기 큐**
```
=== 생산 대기 큐 ===
  순번  주문번호              시료 ID  시료명               생산 수량
    1   ORD-20260612-0003   S-002    GaAs 기판-4인치       15 ea
    2   ORD-20260612-0005   S-001    실리콘 웨이퍼-8인치    10 ea
```

생산 중·대기 모두 없을 시 안내 메시지 출력.

---

## 생산량 계산 참조

실 생산량은 주문 승인(F-003) 시 이미 결정되어 orders.json에 저장하지 않는다.  
조회 시에는 `ProductionCalculator`를 재호출하여 산출한다.

> `실 생산량 = ceil(부족분 / (수율 × 0.9))`  
> 부족분 = quantity (PRODUCING 전환 시점의 재고가 이미 0이므로 전량 생산)

---

## 예외 처리

| 상황 | 처리 |
|------|------|
| PRODUCING 주문 없음 | "생산 중인 주문 없음" 메시지 |
| productionStartTime = 0 | 비정상 데이터로 간주, 오류 로그 출력 후 스킵 |
| SampleRepository 업데이트 실패 | 오류 메시지, 해당 주문 상태 롤백 |

---

## 테스트 케이스

| ID | 시나리오 | 기대 결과 |
|----|----------|----------|
| TC-005-01 | 경과 시간 < 총 생산시간 | 생산 중 표시, 상태 유지 |
| TC-005-02 | 경과 시간 >= 총 생산시간 | CONFIRMED 자동 전환, 재고 증가 후 차감 |
| TC-005-03 | 복수 PRODUCING 주문 | productionStartTime 오름차순 FIFO 정렬 |
| TC-005-04 | 프로그램 재실행 후 진입 | 저장된 productionStartTime 기준 경과 시간 재계산 |
| TC-005-05 | 재실행 직후 (main 진입 시) 생산 완료 조건 충족 | 메인 화면 표시 전 CONFIRMED 자동 전환, 재고 반영 |
| TC-005-06 | 재실행 직후 생산 완료 조건 미충족 | 상태 유지, 메인 화면 정상 표시 |
| TC-005-07 | PRODUCING 주문 없음 | 안내 메시지 출력 |
| TC-005-08 | 앱 실행 중 [5] 재진입 시 추가 완료 조건 충족 | 메뉴 진입 시점 기준 CONFIRMED 추가 전환 |
