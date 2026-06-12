# F-008 생산량 계산 유틸 (Production Calculator)

## 개요
생산 투입량(실 생산량)과 총 생산시간을 계산하는 순수 유틸리티.  
비즈니스 로직과 분리하여 `util/ProductionCalculator`에 격리한다.

---

## 관련 파일

| 레이어 | 파일 |
|--------|------|
| Util | `util/ProductionCalculator.h`, `util/ProductionCalculator.cpp` |

---

## 계산 공식

```
부족분         = 주문 수량 - 현재 재고 할당량
실 생산량      = ceil(부족분 / (수율 × 0.9))
총 생산시간(분) = 평균 생산시간(min/ea) × 실 생산량
총 생산시간(초) = 총 생산시간(분) × 60
```

- 수율(`yield`)과 안전 계수 `0.9`는 투입량 산정에만 사용
- **생산된 전량은 양품**: 실 생산량이 그대로 재고 증가량이 된다

---

## 인터페이스

```cpp
namespace ProductionCalculator {

    // 실 생산량 계산
    // shortage  : 주문 수량 - 현재 재고 (양수 보장)
    // yieldRate : 시료 수율 (0 < yieldRate <= 1.0)
    // 반환값    : ceil(shortage / (yieldRate * 0.9))
    int calcProductionQty(int shortage, double yieldRate);

    // 총 생산시간 계산 (분 단위)
    // avgTime        : 평균 생산시간 (min/ea)
    // productionQty  : calcProductionQty 의 반환값
    int calcTotalTimeMinutes(int avgTime, int productionQty);

    // 총 생산시간 계산 (초 단위) — 생산 완료 시각 비교에 사용
    long long calcTotalTimeSeconds(int avgTime, int productionQty);

} // namespace ProductionCalculator
```

---

## 계산 예시

| 입력 | 계산 과정 | 결과 |
|------|----------|------|
| 부족분=20, 수율=0.92 | ceil(20 / (0.92 × 0.9)) = ceil(24.154…) | **25 ea** |
| 부족분=10, 수율=1.0  | ceil(10 / (1.0 × 0.9)) = ceil(11.11…)  | **12 ea** |
| 부족분=1,  수율=0.5  | ceil(1 / (0.5 × 0.9)) = ceil(2.22…)   | **3 ea** |
| avgTime=120, qty=25 | 120 × 25 | **3,000 분 / 180,000 초** |

---

## 예외 처리

| 상황 | 처리 |
|------|------|
| shortage ≤ 0 | `std::invalid_argument` throw |
| yieldRate ≤ 0.0 또는 > 1.0 | `std::invalid_argument` throw |
| avgTime ≤ 0 | `std::invalid_argument` throw |

---

## 테스트 케이스

| ID | 입력 (부족분, 수율) | 기대 실 생산량 |
|----|--------------------|--------------|
| TC-008-01 | 20, 0.92 | 25 |
| TC-008-02 | 10, 1.0  | 12 |
| TC-008-03 | 1,  0.5  | 3  |
| TC-008-04 | 9,  0.9  | 11 |
| TC-008-05 | 100, 0.8 | 139 |
| TC-008-06 | shortage=0 | `std::invalid_argument` |
| TC-008-07 | yieldRate=0.0 | `std::invalid_argument` |
| TC-008-08 | yieldRate=1.1 | `std::invalid_argument` |
| TC-008-09 | avgTime=120, qty=25 → 분 | 3000 |
| TC-008-10 | avgTime=120, qty=25 → 초 | 180000 |
