# F-007 주문 상태 머신 (Order State Machine)

## 개요
주문(Order)의 상태 전환을 State Machine 방식으로 관리하여 불법 전환을 원천 차단한다.  
모든 상태 변경은 반드시 이 State Machine을 통해서만 수행한다.

---

## 관련 파일

| 레이어 | 파일 |
|--------|------|
| Model | `model/Order.h`, `model/Order.cpp` |

---

## 상태 정의

```cpp
enum class OrderStatus {
    RESERVED,   // 주문 접수
    PRODUCING,  // 생산 중
    CONFIRMED,  // 출고 대기
    RELEASE,    // 출고 완료 (종단)
    REJECTED    // 거절 (종단)
};
```

---

## 허용 전환 테이블

| From | To | 조건 | 호출 주체 |
|------|----|------|----------|
| `RESERVED` | `CONFIRMED` | 승인 + 재고 충분 | `OrderController` |
| `RESERVED` | `PRODUCING` | 승인 + 재고 부족 | `OrderController` |
| `RESERVED` | `REJECTED` | 거절 | `OrderController` |
| `PRODUCING` | `CONFIRMED` | 생산 완료 (자동) | `ProductionController` |
| `CONFIRMED` | `RELEASE` | 출고 처리 | `ReleaseController` |

위 표에 없는 모든 전환은 **불법 전환**으로 예외를 발생시킨다.

---

## 구현 인터페이스

```cpp
class Order {
public:
    // 상태 전환 메서드 — 불법 전환 시 std::logic_error throw
    void approve(bool hasEnoughStock);   // RESERVED → CONFIRMED or PRODUCING
    void reject();                        // RESERVED → REJECTED
    void completeProduction();            // PRODUCING → CONFIRMED
    void release();                       // CONFIRMED → RELEASE

    OrderStatus getStatus() const;

private:
    OrderStatus status_;
    void transitionTo(OrderStatus next); // 전환 유효성 검사 후 상태 변경
};
```

---

## 전환 유효성 검사 로직

```cpp
void Order::transitionTo(OrderStatus next) {
    static const std::map<OrderStatus, std::set<OrderStatus>> allowed = {
        { RESERVED,  { CONFIRMED, PRODUCING, REJECTED } },
        { PRODUCING, { CONFIRMED } },
        { CONFIRMED, { RELEASE } },
        { RELEASE,   { } },   // 종단
        { REJECTED,  { } },   // 종단
    };
    if (allowed.at(status_).count(next) == 0)
        throw std::logic_error("illegal state transition");
    status_ = next;
}
```

---

## 예외 처리

| 상황 | 예외 타입 | 메시지 |
|------|----------|--------|
| 불법 전환 시도 | `std::logic_error` | `"illegal state transition: X -> Y"` |
| 종단 상태에서 전환 시도 | `std::logic_error` | 동일 |

---

## 테스트 케이스

| ID | 시나리오 | 기대 결과 |
|----|----------|----------|
| TC-007-01 | RESERVED → CONFIRMED (합법) | 전환 성공 |
| TC-007-02 | RESERVED → PRODUCING (합법) | 전환 성공 |
| TC-007-03 | RESERVED → REJECTED (합법) | 전환 성공 |
| TC-007-04 | PRODUCING → CONFIRMED (합법) | 전환 성공 |
| TC-007-05 | CONFIRMED → RELEASE (합법) | 전환 성공 |
| TC-007-06 | RESERVED → RELEASE (불법) | `std::logic_error` throw |
| TC-007-07 | CONFIRMED → PRODUCING (불법) | `std::logic_error` throw |
| TC-007-08 | RELEASE → CONFIRMED (불법, 종단) | `std::logic_error` throw |
| TC-007-09 | REJECTED → RESERVED (불법, 종단) | `std::logic_error` throw |
| TC-007-10 | PRODUCING → REJECTED (불법) | `std::logic_error` throw |
