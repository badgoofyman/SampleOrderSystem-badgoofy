# Phase 2 검토 체크리스트

**브랜치**: `phase/2-controller-view`  
**검토 시점**: PR 리뷰 전 로컬에서 직접 확인

---

## 1. 빌드 및 테스트 자동 확인

> Visual Studio에서 Debug | x64 빌드 후 출력 창 확인

- [ ] **Debug|x64 빌드 성공** — 오류·경고 0건
- [ ] **Release|x64 빌드 성공** — main.cpp 포함, 테스트 파일 제외
- [ ] **Post-Build Event 단위 테스트 자동 실행** — 출력 창에 테스트 결과 출력 확인
- [ ] **모든 테스트 PASSED** — `[  PASSED  ] 71 tests.` 메시지 확인
- [ ] **실패 테스트 0건** — `[  FAILED  ]` 항목 없음 확인

---

## 2. 코드 구조 검토

### 2-1. 파일 구성
- [ ] `model/ProductionLine.h`, `model/ProductionLine.cpp` 존재
- [ ] `model/StockStatus.h` 존재 (레이어 중립 DTO)
- [ ] `repository/IOrderRepository.h` 존재
- [ ] `controller/` 아래 4개 Controller (h/cpp) 존재
- [ ] `view/` 아래 5개 View (h/cpp) 존재
- [ ] `test/MockRepositories.h` 존재
- [ ] `test/` 아래 5개 신규 테스트 파일 존재
- [ ] `main.cpp` 존재

### 2-2. 파일 인코딩
- [ ] 한글 string literal 포함 `.cpp` / `.h` 파일이 **UTF-8 with BOM**으로 저장됨  
  대상: `main.cpp`, `MenuView.cpp`, `SampleView.h/.cpp`, `OrderView.h/.cpp`, `MonitorView.h/.cpp`, `ProductionView.cpp`, `SampleController.cpp`, `OrderController.cpp`, `ReleaseController.cpp`

### 2-3. MVC 의존 방향
- [ ] `model/` 파일이 `view/` / `controller/` 헤더를 `#include` 하지 않음
- [ ] `repository/` 파일이 `controller/` / `view/` 헤더를 `#include` 하지 않음
- [ ] `StockStatus`가 `model/StockStatus.h`에 정의됨 (View 헤더에 없음)
- [ ] `ProductionController`가 `std::istream&`을 생성자 인자로 받지 않음 (미사용 제거됨)

---

## 3. 도메인 모델 검토

### 3-1. ProductionJob 구조체
- [ ] `orderNo`, `sampleId`, `productionQty`, `totalTimeSeconds`, `startTime` 필드 모두 존재

### 3-2. ProductionLine 클래스
- [ ] 내부 컨테이너가 `std::deque` (vector 아님) — `completeAndAdvance()` O(1) 보장
- [ ] `current()` 빈 상태에서 `std::runtime_error` throw
- [ ] `isCurrentComplete()` 조건: `time(nullptr) >= startTime + totalTimeSeconds`
- [ ] `completeAndAdvance()` 구현이 `pop_front()` 사용

### 3-3. Order 필드 확장
- [ ] `Order::productionQty` 필드 존재 (기본값 0)
- [ ] `productionQty` JSON 직렬화/역직렬화 포함

### 3-4. IOrderRepository
- [ ] `IRepository<Order>`를 상속
- [ ] `findByStatus(OrderStatus)` 순수 가상 메서드 포함
- [ ] `OrderRepository`가 `IOrderRepository`를 상속하고 `override` 표시

---

## 4. Controller 검토

### 4-1. SampleController
- [ ] `manageSamples()` 서브메뉴 루프: [1] 등록 [2] 목록 [3] 검색 [0] 뒤로
- [ ] `showMonitoring(IOrderRepository&)` — REJECTED 주문 집계 제외 확인
- [ ] 재고 상태 판단 로직:
  - `stock == 0` → **고갈**
  - `pending > stock && stock > 0` → **부족**
  - `pending <= stock && stock > 0` → **여유**

### 4-2. OrderController
- [ ] 주문번호 형식: `ORD-YYYYMMDD-NNNN` (4자리 시퀀스, 0-패딩)
- [ ] 승인 시 재고 충분: `approve(true)` + `sample.stock -= qty` + `sampleRepo.update()`
- [ ] 승인 시 재고 부족: `calcProductionQty` → `line.enqueue()` + `order.productionQty` 저장 + `approve(false)`
- [ ] 재고 부족 승인 시 `productionStartTime` 설정:
  - 라인 비어있음: `time(nullptr)`
  - 라인에 잡 있음: `lastJob.startTime + lastJob.totalTimeSeconds`
- [ ] 거절: `reject()` + `orderRepo.update()`

### 4-3. ProductionController
- [ ] 완료된 잡 처리: `completeProduction()` → CONFIRMED 전환
- [ ] 재고 반영: `sample.stock += productionQty - quantity`
- [ ] `std::istream&`을 생성자 인자로 받지 않음

### 4-4. ReleaseController
- [ ] `release()` + `releasedAt = time(nullptr)` 설정
- [ ] `orderRepo.update()` 호출

---

## 5. View 검토

### 5-1. 공통 설계 원칙
- [ ] 모든 View 메서드가 `static`
- [ ] 모든 메서드가 `std::istream&` / `std::ostream&` 인자를 통해 입출력 — 하드코딩된 `std::cin/cout` 없음

### 5-2. OrderView
- [ ] `printOrderList` 함수명 사용 (구버전 `printReservedList` 없음)

### 5-3. MonitorView
- [ ] `StockStatus` 구조체를 `#include "../model/StockStatus.h"`로 가져옴 (자체 정의 없음)

---

## 6. main.cpp 검토

- [ ] `SetConsoleOutputCP(CP_UTF8)` + `SetConsoleCP(CP_UTF8)` 호출 위치: `main()` 시작 직후
- [ ] `buildProductionLine()` 파라미터가 `IOrderRepository&` (구체 타입 아님)
- [ ] PRODUCING 주문을 `productionStartTime` **오름차순** 정렬 후 `line.enqueue()` 실행
- [ ] 메인 루프 메뉴 번호 [1]~[6], [0] 종료 정상 매핑

---

## 7. 테스트 커버리지 검토

### 7-1. ProductionLine (9개)
- [ ] isEmpty/size 초기화 상태 확인
- [ ] enqueue 후 size 증가
- [ ] current() 빈 상태 throw
- [ ] current() FIFO 순서
- [ ] isCurrentComplete — 미완료/완료 양쪽
- [ ] completeAndAdvance — 첫 항목 제거 후 다음 항목
- [ ] completeAndAdvance — 마지막 후 isEmpty
- [ ] jobs() 전체 반환

### 7-2. SampleController (5개)
- [ ] listSamples → findAll 호출 확인
- [ ] registerSample → save 호출 확인
- [ ] searchByName — 일치 결과 출력
- [ ] searchByName — 불일치 시 "없습니다" 출력
- [ ] showMonitoring — 재고 상태 집계 (`여유`)

### 7-3. OrderController (6개)
- [ ] placeOrder — 유효하지 않은 시료 ID → 오류 메시지
- [ ] placeOrder — 유효한 시료 → save 호출
- [ ] processApproval — 충분 재고 → CONFIRMED 출력
- [ ] processApproval — 부족 재고 → PRODUCING + enqueue
- [ ] processApproval — 거절 → "거절" 출력
- [ ] processApproval — RESERVED 없음 → "없습니다" 출력

### 7-4. ProductionController (3개)
- [ ] 빈 라인 → "없습니다" 출력
- [ ] 완료된 잡 → CONFIRMED 전환 + update 호출 + 라인 비워짐
- [ ] 진행중 잡 → 변화 없음, 주문번호 출력

### 7-5. ReleaseController (3개)
- [ ] CONFIRMED 없음 → "없습니다" 출력
- [ ] 유효 주문번호 → "출고" 출력 + update 호출
- [ ] 무효 주문번호 → "찾을 수 없습니다" 출력

---

## 8. 전체 흐름 수동 실행 확인

시료 등록 → 주문 → 승인 → 출고 전체 시나리오:

### 시나리오 A — 재고 충분
1. [1] 시료 등록: S-001, 재고 50
2. [2] 시료 주문: S-001, 수량 10
3. [3] 주문 승인: 승인(1) → 상태 CONFIRMED 확인
4. [4] 모니터링: S-001 재고 40, 상태 "여유" 확인
5. [6] 출고 처리 → RELEASE 확인
6. 앱 종료 후 재실행 → [4] 모니터링에 RELEASE 1건 확인

### 시나리오 B — 재고 부족
1. [1] 시료 등록: S-002, 재고 5, 수율 0.9, 생산시간 60분
2. [2] 시료 주문: S-002, 수량 20
3. [3] 주문 승인: 승인(1) → 상태 PRODUCING 확인
4. [4] 모니터링: S-002 상태 "부족" 확인
5. [5] 생산라인 조회: ORD-XXXXXXXX-0001 생산 중 표시 확인

### 시나리오 C — 앱 재시작 후 ProductionLine 재구성
1. 시나리오 B 이후 앱 종료
2. 재실행 → [5] 생산라인 조회 → 동일 잡이 복원됨 확인

### 시나리오 D — 주문 거절
1. [2] 시료 주문 → [3] 거절(0) → 상태 REJECTED 확인
2. [4] 모니터링 집계에서 REJECTED 주문 미포함 확인

---

## 9. JSON 영속성 검토

`data/orders.json` 직접 열어 확인:
```json
[
  {
    "orderNo": "ORD-20260612-0001",
    "sampleId": "S-001",
    "customerName": "삼성전자",
    "quantity": 20,
    "productionQty": 18,
    "status": "PRODUCING",
    "productionStartTime": 1749689400,
    "releasedAt": 0
  }
]
```
- [ ] `productionQty` 필드가 숫자로 저장됨
- [ ] `status` 문자열 ("PRODUCING" 등) 정상 저장
- [ ] `productionStartTime` epoch 정수로 저장

---

## 10. Git 이력 검토

`git log --oneline` 으로 확인:

- [ ] TDD 커밋 순서 준수 — `test:` 커밋이 `feat:` 커밋보다 앞에 위치
- [ ] `chore:` 커밋이 마지막 그룹에 위치
- [ ] 불필요한 임시 커밋 없음
- [ ] 커밋 메시지에 한글 포함 시 깨짐 없음

---

## 11. 코드 품질 검토

- [ ] 함수 길이 30줄 이내 (단일 책임 원칙)
- [ ] 모든 Controller가 `IRepository<Sample>&` / `IOrderRepository&` 인터페이스를 통해 Repository 접근
- [ ] `ProductionLine`의 컨테이너가 `std::deque` (O(1) pop_front)
- [ ] `StockStatus`가 View 헤더가 아닌 `model/StockStatus.h`에 정의됨
- [ ] `buildProductionLine()` 파라미터가 `IOrderRepository&`

---

## 12. PR 발행 전 최종 확인

- [ ] 위 1~11번 항목 전체 통과
- [ ] `phase/2-controller-view` 브랜치가 최신 `main`과 충돌 없음
- [ ] PR 본문에 테스트 결과 포함
- [ ] PR 제목: `[Phase 2] Controller + View 계층 구현 — 콘솔 애플리케이션 완성`
