# F-006 출고 처리 (Release)

## 개요
출고 대기(CONFIRMED) 상태 주문을 선택하여 출고 완료(RELEASE)로 처리하는 기능.  
메인 메뉴 **[6] 출고 처리** 에 해당한다.

---

## 관련 파일

| 레이어 | 파일 |
|--------|------|
| Model | `model/Order.h` |
| Repository | `repository/OrderRepository.h`, `repository/OrderRepository.cpp` |
| Controller | `controller/ReleaseController.h`, `controller/ReleaseController.cpp` |
| View | `view/OrderView.h`, `view/OrderView.cpp` |
| DB | `data/orders.json` |

---

## 기능 상세

### F-006-1 CONFIRMED 목록 표시

- `OrderRepository::findByStatus(CONFIRMED)` 로 조회
- 주문번호 / 시료 ID / 시료명 / 고객명 / 수량 테이블 출력
- 목록 없을 시 안내 메시지 출력 후 메뉴 복귀

### F-006-2 출고 실행 흐름

1. 처리할 주문번호 입력
2. 입력된 주문번호가 CONFIRMED 목록에 존재하는지 확인
3. 상태: CONFIRMED → RELEASE
4. `OrderRepository::update()` → orders.json 저장
5. 출고 완료 메시지 출력 (초록)

---

## 상태 전환

```
CONFIRMED → RELEASE
```

- 재고 변경 없음 (재고는 CONFIRMED 전환 시점에 이미 차감됨)
- RELEASE 이후 추가 상태 전환 없음 (종단 상태)

---

## 예외 처리

| 상황 | 처리 |
|------|------|
| CONFIRMED 주문 없음 | 안내 메시지 후 메뉴 복귀 |
| 목록에 없는 주문번호 입력 | 오류 메시지(빨강) 후 재입력 |
| JSON 쓰기 실패 | 오류 메시지, 상태 롤백 |

---

## 테스트 케이스

| ID | 시나리오 | 기대 결과 |
|----|----------|----------|
| TC-006-01 | 정상 출고 처리 | RELEASE 전환, orders.json 저장 |
| TC-006-02 | CONFIRMED 목록 없음 | 안내 메시지 출력 |
| TC-006-03 | 잘못된 주문번호 입력 | 오류 메시지, 재입력 요청 |
| TC-006-04 | 출고 후 모니터링 확인 | RELEASE 건수 1 증가, CONFIRMED 1 감소 |
