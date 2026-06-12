# Phase 1 구현 계획 — 기반 인프라

**브랜치**: `phase/1-foundation`  
**참조 Feature**: F-007, F-008, F-009  
**목표**: 도메인 모델, State Machine, Repository, ProductionCalculator 구현 및 단위 테스트 완비

---

## 구현 대상 파일

```
model/
  Sample.h / Sample.cpp
  Order.h  / Order.cpp
repository/
  IRepository.h
  SampleRepository.h / SampleRepository.cpp
  OrderRepository.h  / OrderRepository.cpp
util/
  ProductionCalculator.h / ProductionCalculator.cpp
test/
  SampleTest.cpp
  OrderStateMachineTest.cpp
  ProductionCalculatorTest.cpp
  SampleRepositoryTest.cpp
  OrderRepositoryTest.cpp
data/                          ← .gitkeep 으로 디렉터리만 커밋
  .gitkeep
```

---

## 사전 준비

### nlohmann/json 도입

`repository/` 에서 JSON 파싱에 사용할 헤더 전용 라이브러리.  
`thirdparty/nlohmann/json.hpp` 단일 헤더 파일로 배치한다.

```
thirdparty/
  nlohmann/
    json.hpp      ← 단일 헤더 (v3.x)
```

vcxproj `Include` 경로에 `thirdparty/` 추가 필요.

---

## Step별 구현 순서 및 커밋 계획

### Step 1 — `Sample` 도메인 모델

**목표**: Sample 구조체 정의  
**파일**: `model/Sample.h`, `model/Sample.cpp`

```cpp
struct Sample {
    std::string id;
    std::string name;
    int         avgProductionTime;  // min/ea, 1 이상
    double      yield;              // 0.0 < yield <= 1.0
    int         stock;              // 0 이상
};
```

**커밋**
```
feat: Sample 도메인 모델 정의
```

---

### Step 2 — `ProductionCalculator` (TDD)

**목표**: 실 생산량·총 생산시간 계산 함수  
**파일**: `util/ProductionCalculator.h`, `util/ProductionCalculator.cpp`  
**테스트**: `test/ProductionCalculatorTest.cpp`

#### 🔴 RED
```
test: ProductionCalculator 단위 테스트 작성
```
작성할 테스트:
- `calcProductionQty(20, 0.92)` → 25
- `calcProductionQty(10, 1.0)` → 12
- `calcProductionQty(1, 0.5)` → 3
- `calcProductionQty(9, 0.9)` → 11
- `calcProductionQty(100, 0.8)` → 139
- `shortage <= 0` → `std::invalid_argument`
- `yieldRate <= 0.0` → `std::invalid_argument`
- `yieldRate > 1.0` → `std::invalid_argument`
- `calcTotalTimeMinutes(120, 25)` → 3000
- `calcTotalTimeSeconds(120, 25)` → 180000
- `avgTime <= 0` → `std::invalid_argument`

#### 🟢 GREEN
```
feat: ProductionCalculator 구현
```

#### 🔵 BLUE (필요 시)
```
refactor: ProductionCalculator 코드 정리
```

---

### Step 3 — `OrderStatus` + `Order` 도메인 모델 (TDD)

**목표**: Order 구조체 + State Machine 구현  
**파일**: `model/Order.h`, `model/Order.cpp`  
**테스트**: `test/OrderStateMachineTest.cpp`

#### 🔴 RED
```
test: Order State Machine 단위 테스트 작성
```
작성할 테스트:

합법 전환 (5가지):
- `RESERVED → CONFIRMED` (`approve(true)`)
- `RESERVED → PRODUCING` (`approve(false)`)
- `RESERVED → REJECTED` (`reject()`)
- `PRODUCING → CONFIRMED` (`completeProduction()`)
- `CONFIRMED → RELEASE` (`release()`)

불법 전환 (예외 발생 확인):
- `RESERVED → RELEASE` → `std::logic_error`
- `CONFIRMED → PRODUCING` → `std::logic_error`
- `RELEASE → CONFIRMED` (종단) → `std::logic_error`
- `REJECTED → RESERVED` (종단) → `std::logic_error`
- `PRODUCING → REJECTED` → `std::logic_error`

#### 🟢 GREEN
```
feat: Order 도메인 모델 및 State Machine 구현
```

Order 구조:
```cpp
struct Order {
    std::string  orderNo;
    std::string  sampleId;
    std::string  customerName;
    int          quantity;
    OrderStatus  status;
    time_t       productionStartTime;  // 0: 미사용
    time_t       releasedAt;           // 0: 미사용
};
```

State Machine (`transitionTo` private 메서드):
```cpp
static const std::map<OrderStatus, std::set<OrderStatus>> allowed = {
    { RESERVED,  { CONFIRMED, PRODUCING, REJECTED } },
    { PRODUCING, { CONFIRMED } },
    { CONFIRMED, { RELEASE   } },
    { RELEASE,   { } },
    { REJECTED,  { } },
};
```

#### 🔵 BLUE (필요 시)
```
refactor: Order State Machine 코드 정리
```

---

### Step 4 — `IRepository` 인터페이스

**목표**: 저장소 추상화 인터페이스 정의  
**파일**: `repository/IRepository.h`

```cpp
template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;
    virtual std::vector<T>        findAll()                         const = 0;
    virtual std::optional<T>      findById(const std::string& id)  const = 0;
    virtual void                  save(const T& entity)                   = 0;
    virtual void                  update(const T& entity)                 = 0;
    virtual void                  remove(const std::string& id)           = 0;
};
```

**커밋**
```
feat: IRepository 인터페이스 정의
```

---

### Step 5 — `SampleRepository` (TDD)

**목표**: 시료 JSON CRUD 구현  
**파일**: `repository/SampleRepository.h`, `repository/SampleRepository.cpp`  
**테스트**: `test/SampleRepositoryTest.cpp`

#### 🔴 RED
```
test: SampleRepository 단위 테스트 작성
```
작성할 테스트 (실제 임시 파일 사용):
- `save()` 후 `findAll()` → 저장한 항목 반환
- `findById()` 존재하는 ID → 해당 Sample 반환
- `findById()` 없는 ID → `std::nullopt`
- `update()` 후 `findById()` → 변경된 값 반환
- `remove()` 후 `findById()` → `std::nullopt`
- 파일 미존재 시 `findAll()` → 빈 벡터, 파일 생성

#### 🟢 GREEN
```
feat: SampleRepository JSON CRUD 구현
```

구현 포인트:
- `nlohmann::json` 사용
- `id` 필드로 항목 식별
- 변경 즉시 전체 파일 재기록

#### 🔵 BLUE (필요 시)
```
refactor: SampleRepository 코드 정리
```

---

### Step 6 — `OrderRepository` (TDD)

**목표**: 주문 JSON CRUD 구현  
**파일**: `repository/OrderRepository.h`, `repository/OrderRepository.cpp`  
**테스트**: `test/OrderRepositoryTest.cpp`

#### 🔴 RED
```
test: OrderRepository 단위 테스트 작성
```
작성할 테스트:
- `save()` 후 `findAll()` → 저장한 항목 반환
- `findById()` 존재하는 orderNo → 해당 Order 반환
- `update()` status 변경 후 → 변경된 status 반환
- `productionStartTime` 저장·로드 → epoch 값 정확히 복원
- `releasedAt` 저장·로드 → epoch 값 정확히 복원
- `findByStatus(RESERVED)` → 해당 상태 주문만 반환
- `remove()` 후 `findById()` → `std::nullopt`
- 파일 미존재 시 `findAll()` → 빈 벡터, 파일 생성

#### 🟢 GREEN
```
feat: OrderRepository JSON CRUD 구현
```

추가 메서드 (`IRepository` 확장):
```cpp
std::vector<Order> findByStatus(OrderStatus status) const;
```

#### 🔵 BLUE (필요 시)
```
refactor: OrderRepository 코드 정리
```

---

### Step 7 — `data/` 디렉터리 및 vcxproj 파일 등록

**목표**: 구현 파일을 VS 프로젝트에 등록, data 디렉터리 생성  
**파일**: `SampleOrderSystem.vcxproj`, `SampleOrderSystem.vcxproj.filters`, `data/.gitkeep`

**커밋**
```
chore: 구현 파일 vcxproj 등록 및 data 디렉터리 추가
```

---

## E2E 체크리스트 (PR 발행 전 확인)

- [ ] Debug 빌드 성공, 단위 테스트 전체 통과 (Post-Build Event 자동 실행)
- [ ] `data/` 없는 상태에서 실행 시 `samples.json` / `orders.json` 자동 생성 확인
- [ ] Sample 저장 후 재실행 시 동일 데이터 복원 확인
- [ ] Order 상태 전환 5가지 정상 동작 확인
- [ ] 불법 전환 시 `std::logic_error` 발생 확인
- [ ] `ceil(부족분 / (수율 × 0.9))` 수치 결과 검증
- [ ] `productionStartTime` / `releasedAt` JSON 저장·복원 확인

---

## PR 정보

- **제목**: `[Phase 1] 기반 인프라 — 도메인 모델 + Repository + ProductionCalculator`
- **base**: `main`
- **compare**: `phase/1-foundation`

---

## 커밋 순서 요약

| 순서 | 단계 | 커밋 메시지 |
|------|------|------------|
| 1 | Step 1 | `feat: Sample 도메인 모델 정의` |
| 2 | Step 2 RED | `test: ProductionCalculator 단위 테스트 작성` |
| 3 | Step 2 GREEN | `feat: ProductionCalculator 구현` |
| 4 | Step 3 RED | `test: Order State Machine 단위 테스트 작성` |
| 5 | Step 3 GREEN | `feat: Order 도메인 모델 및 State Machine 구현` |
| 6 | Step 4 | `feat: IRepository 인터페이스 정의` |
| 7 | Step 5 RED | `test: SampleRepository 단위 테스트 작성` |
| 8 | Step 5 GREEN | `feat: SampleRepository JSON CRUD 구현` |
| 9 | Step 6 RED | `test: OrderRepository 단위 테스트 작성` |
| 10 | Step 6 GREEN | `feat: OrderRepository JSON CRUD 구현` |
| 11 | Step 7 | `chore: 구현 파일 vcxproj 등록 및 data 디렉터리 추가` |
