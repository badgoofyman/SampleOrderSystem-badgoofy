# F-002 시료 주문 접수 (Order Placement)

## 개요
주문담당자가 고객 요청을 기반으로 주문서를 작성하는 기능.  
메인 메뉴 **[2] 시료 주문** 에 해당한다.

---

## 관련 파일

| 레이어 | 파일 |
|--------|------|
| Model | `model/Order.h`, `model/Order.cpp` |
| Repository | `repository/IRepository.h`, `repository/OrderRepository.h`, `repository/OrderRepository.cpp` |
| Controller | `controller/OrderController.h`, `controller/OrderController.cpp` |
| View | `view/OrderView.h`, `view/OrderView.cpp` |
| DB | `data/orders.json` |

---

## 기능 상세

### F-002-1 주문 접수 흐름

1. 고객명 입력
2. 시료 ID 입력
   - `SampleRepository::findById()` 로 존재 여부 확인
   - 존재하지 않으면 오류 출력 후 재입력 요청
3. 주문 수량 입력 (1 이상 정수)
4. 주문번호 자동 생성 (F-002-2 참조)
5. 상태 **RESERVED**, `productionStartTime = 0` 으로 Order 객체 생성
6. `OrderRepository::save()` 호출 → `data/orders.json` 즉시 저장
7. 접수 완료 메시지 및 생성된 주문번호 출력 (초록)

### F-002-2 주문번호 자동 생성 규칙

```
형식: ORD-YYYYMMDD-NNNN
예시: ORD-20260612-0001
```

- `YYYYMMDD`: 주문 접수 당일 날짜
- `NNNN`: `orders.json` 에서 동일 날짜(`ORD-YYYYMMDD-` 접두사 일치) 주문 건수 + 1, 4자리 zero-padding
- 카운터는 별도 파일 없이 `orders.json` 에서 파생

---

## 데이터 모델

```cpp
enum class OrderStatus {
    RESERVED,
    PRODUCING,
    CONFIRMED,
    RELEASE,
    REJECTED
};

struct Order {
    std::string  orderNo;              // "ORD-20260612-0001"
    std::string  sampleId;             // "S-001"
    std::string  customerName;         // "삼성전자"
    int          quantity;             // 20
    OrderStatus  status;               // RESERVED
    time_t       productionStartTime;  // 0 (미생산 상태)
};
```

---

## 입력 유효성 검사

| 필드 | 규칙 |
|------|------|
| 고객명 | 빈 문자열 불가 |
| 시료 ID | samples.json 에 존재해야 함 |
| 주문 수량 | 1 이상 정수 |

---

## 예외 처리

| 상황 | 처리 |
|------|------|
| 존재하지 않는 시료 ID | 오류 메시지(빨강) 후 재입력 |
| 수량 0 이하 | 오류 메시지(빨강) 후 재입력 |
| 빈 고객명 | 오류 메시지(빨강) 후 재입력 |
| JSON 쓰기 실패 | 오류 메시지 출력 후 접수 취소 |

---

## 테스트 케이스

| ID | 시나리오 | 기대 결과 |
|----|----------|----------|
| TC-002-01 | 정상 주문 접수 | orders.json 저장, 주문번호 출력 |
| TC-002-02 | 존재하지 않는 시료 ID | 오류 메시지, 재입력 요청 |
| TC-002-03 | 수량 0 입력 | 오류 메시지, 재입력 요청 |
| TC-002-04 | 빈 고객명 입력 | 오류 메시지, 재입력 요청 |
| TC-002-05 | 당일 첫 주문 | 순번 0001 생성 |
| TC-002-06 | 당일 N번째 주문 | 순번 N+1 생성 |
| TC-002-07 | 날짜 변경 후 첫 주문 | 새 날짜로 순번 0001 생성 |
