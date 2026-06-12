# Phase 1 검토 체크리스트

**브랜치**: `phase/1-foundation`  
**검토 시점**: PR 리뷰 전 로컬에서 직접 확인

---

## 1. 빌드 및 테스트 자동 확인

> Visual Studio에서 Debug | x64 빌드 후 출력 창 확인

- [ ] **Debug 빌드 성공** — 오류·경고 0건
- [ ] **Post-Build Event 단위 테스트 자동 실행** — 출력 창에 테스트 결과 출력 확인
- [ ] **모든 테스트 PASSED** — `[  PASSED  ] N tests.` 메시지 확인
- [ ] **실패 테스트 0건** — `[  FAILED  ]` 항목 없음 확인

---

## 2. 코드 구조 검토

### 2-1. 파일 구성
- [ ] `model/Sample.h`, `model/Sample.cpp` 존재
- [ ] `model/Order.h`, `model/Order.cpp` 존재
- [ ] `repository/IRepository.h` 존재
- [ ] `repository/SampleRepository.h`, `repository/SampleRepository.cpp` 존재
- [ ] `repository/OrderRepository.h`, `repository/OrderRepository.cpp` 존재
- [ ] `util/ProductionCalculator.h`, `util/ProductionCalculator.cpp` 존재
- [ ] `test/` 아래 5개 테스트 파일 존재
- [ ] `data/.gitkeep` 존재 (빈 디렉터리 추적용)
- [ ] `util/JsonUtil.h`, `util/JsonUtil.cpp` 존재

### 2-2. 파일 인코딩
- [ ] 한글이 포함된 `.h` / `.cpp` 파일이 **UTF-8 with BOM** 으로 저장됨  
  확인 방법: VS에서 해당 파일 열기 → 파일 > 다른 이름으로 저장 > 인코딩 드롭다운에서 `유니코드(서명 있는 UTF-8, 코드 페이지 65001)` 표시 확인  
  (Phase 1 소스 파일은 ASCII 전용이므로 Phase 2 이후 뷰 파일 추가 시 확인)

### 2-3. MVC 의존 방향
- [ ] `model/` 파일이 `repository/` 또는 `controller/` 헤더를 `#include` 하지 않음
- [ ] `repository/` 파일이 `controller/` 헤더를 `#include` 하지 않음
- [ ] `IRepository.h`가 `Sample.h` / `Order.h` 에만 의존

---

## 3. 도메인 모델 검토

### 3-1. Sample 구조체
- [ ] `id`, `name`, `avgProductionTime`, `yield`, `stock` 필드 모두 존재
- [ ] 필드 타입이 PRD와 일치 (`int avgProductionTime`, `double yield`, `int stock`)

### 3-2. Order 구조체
- [ ] `orderNo`, `sampleId`, `customerName`, `quantity`, `status`, `productionStartTime`, `releasedAt` 필드 모두 존재
- [ ] `productionStartTime`, `releasedAt` 의 기본값이 `0`

### 3-3. OrderStatus 열거형
- [ ] `RESERVED`, `PRODUCING`, `CONFIRMED`, `RELEASE`, `REJECTED` 5가지 값 정의

---

## 4. State Machine 검토

### 4-1. 허용 전환 직접 호출 확인
콘솔 또는 테스트 결과로 아래 전환이 예외 없이 실행되는지 확인:

- [ ] `approve(true)` → `RESERVED → CONFIRMED`
- [ ] `approve(false)` → `RESERVED → PRODUCING`
- [ ] `reject()` → `RESERVED → REJECTED`
- [ ] `completeProduction()` → `PRODUCING → CONFIRMED`
- [ ] `release()` → `CONFIRMED → RELEASE`

### 4-2. 불법 전환 예외 확인
테스트 결과에서 아래 케이스가 `std::logic_error` throw로 처리되는지 확인:

- [ ] `RELEASE` 상태에서 어떤 전환도 불가
- [ ] `REJECTED` 상태에서 어떤 전환도 불가
- [ ] `CONFIRMED → PRODUCING` 불가
- [ ] `RESERVED → RELEASE` 직접 전환 불가

---

## 5. ProductionCalculator 검토

### 5-1. 수치 검증 (테스트 결과 확인)
- [ ] `calcProductionQty(20, 0.92)` = **25**
- [ ] `calcProductionQty(10, 1.0)` = **12**
- [ ] `calcProductionQty(1, 0.5)` = **3**
- [ ] `calcProductionQty(9, 0.9)` = **12**
- [ ] `calcProductionQty(100, 0.8)` = **139**

### 5-2. 예외 처리 확인
- [ ] `shortage <= 0` → `std::invalid_argument` throw
- [ ] `yieldRate <= 0.0` → `std::invalid_argument` throw
- [ ] `yieldRate > 1.0` → `std::invalid_argument` throw
- [ ] `avgTime <= 0` → `std::invalid_argument` throw

### 5-3. 시간 계산 확인
- [ ] `calcTotalTimeMinutes(120, 25)` = **3,000**
- [ ] `calcTotalTimeSeconds(120, 25)` = **180,000**

---

## 6. Repository 동작 검토

### 6-1. data/ 디렉터리 자동 생성
- [ ] `data/` 디렉터리 삭제 후 테스트 실행 → `data/samples.json`, `data/orders.json` 자동 생성 확인
- [ ] 생성된 파일 내용이 `[]` (빈 배열) 인지 확인

### 6-2. SampleRepository 영속성 확인
아래 순서로 직접 실행:
1. `SampleRepository`에 Sample 1개 저장
2. 프로그램 종료
3. 재실행 후 `findAll()` 호출

- [ ] 재실행 후 동일 데이터가 복원됨
- [ ] `samples.json` 파일을 메모장으로 열어 JSON 형식이 올바른지 확인

### 6-3. OrderRepository 영속성 확인
- [ ] Order 저장 후 재실행 시 동일 데이터 복원
- [ ] `productionStartTime` 값(epoch 정수)이 JSON에 정확히 저장·복원됨
- [ ] `releasedAt` 값이 JSON에 정확히 저장·복원됨
- [ ] `status` 값이 문자열(`"RESERVED"` 등)로 저장되는지 확인
- [ ] `findByStatus(RESERVED)` 가 해당 상태 주문만 반환하는지 확인

---

## 7. JSON 스키마 검토

`data/samples.json` 직접 열어 확인:
```json
[
  {
    "id": "S-001",
    "name": "실리콘 웨이퍼-8인치",
    "avgProductionTime": 120,
    "yield": 0.92,
    "stock": 50
  }
]
```
- [ ] 필드명 오탈자 없음 (`avgProductionTime`, `yield` 등)
- [ ] `yield` 값이 문자열이 아닌 **숫자**로 저장됨

`data/orders.json` 직접 열어 확인:
```json
[
  {
    "orderNo": "ORD-20260612-0001",
    "sampleId": "S-001",
    "customerName": "삼성전자",
    "quantity": 20,
    "status": "PRODUCING",
    "productionStartTime": 1749689400,
    "releasedAt": 0
  }
]
```
- [ ] `status` 가 문자열로 저장됨
- [ ] `productionStartTime`, `releasedAt` 이 정수로 저장됨

---

## 8. Git 이력 검토

PR을 열기 전 커밋 이력 확인 (`git log --oneline`):

- [ ] 커밋 메시지 접두사 규칙 준수 (`test:` → `feat:` 순서)
- [ ] `test:` 커밋이 반드시 `feat:` 커밋보다 앞에 위치
- [ ] `chore:` 커밋이 마지막에 위치
- [ ] 불필요한 임시 커밋 없음 (WIP, 오타 수정 등은 squash 권장)
- [ ] 커밋 메시지에 한글 포함 시 깨짐 없음

---

## 9. 코드 품질 검토

- [ ] 함수 길이가 30줄 이내 (단일 책임 원칙)
- [ ] 매직 넘버 `0.9` (안전 계수)가 상수로 정의되거나 주석으로 의미 명시
- [ ] `transitionTo()` 의 허용 전환 테이블이 코드에서 명확히 식별 가능
- [ ] `IRepository` 소멸자가 `virtual`로 선언됨
- [ ] `findById()` 반환 타입이 `std::optional<T>` 사용
- [ ] 테스트 파일에서 테스트 케이스 이름이 시나리오를 설명하는 명확한 이름인지 확인

---

## 10. PR 발행 전 최종 확인

- [ ] 위 1~9번 항목 전체 통과
- [ ] `phase/1-foundation` 브랜치가 최신 `main`과 충돌 없음
- [ ] PR 본문에 이 체크리스트 결과 포함
- [ ] PR 제목: `[Phase 1] 기반 인프라 — 도메인 모델 + Repository + ProductionCalculator`
