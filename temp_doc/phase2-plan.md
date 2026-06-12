# Phase 2 구현 계획 — Controller + View 계층

**브랜치**: `phase/2-controller-view`  
**목표**: Phase 1 기반 위에 Controller · View 계층을 구현하여 실제 동작하는 콘솔 애플리케이션을 완성한다.

---

## 구현 대상 파일

```
model/
  ProductionLine.h / ProductionLine.cpp   ← 생산라인 FIFO 큐
  StockStatus.h                           ← 모니터링용 재고 상태 DTO

repository/
  IOrderRepository.h                      ← findByStatus 포함 확장 인터페이스
  OrderRepository.h / .cpp               ← IOrderRepository 상속으로 변경 + productionQty 직렬화

controller/
  SampleController.h / .cpp              ← 시료 관리 (메뉴 1, 4)
  OrderController.h  / .cpp              ← 주문 접수·승인/거절 (메뉴 2, 3)
  ProductionController.h / .cpp          ← 생산라인 조회 (메뉴 5)
  ReleaseController.h / .cpp             ← 출고 처리 (메뉴 6)

view/
  MenuView.h / .cpp                      ← 메인 메뉴 출력 + 입력
  SampleView.h / .cpp                    ← 시료 서브메뉴, 목록 테이블, 등록/검색
  OrderView.h  / .cpp                    ← 주문 입력, 주문 목록 테이블, 승인/거절 UI
  MonitorView.h / .cpp                   ← 주문 상태별 집계, 재고 현황 테이블
  ProductionView.h / .cpp                ← 생산라인 현황 (생산중/대기, 완료예정시간)

test/
  MockRepositories.h                     ← MockSampleRepository, MockOrderRepository
  ProductionLineTest.cpp
  SampleControllerTest.cpp
  OrderControllerTest.cpp
  ProductionControllerTest.cpp
  ReleaseControllerTest.cpp

main.cpp                                 ← 진입점 (ProductionLine 재구성 + 메인 루프)
```

---

## Step별 구현 순서 및 커밋 계획

### Step 0 — 사전 준비: 도메인 확장 + 인터페이스 추가

**목표**: Phase 1 도메인·Repository를 Phase 2 요구사항에 맞게 확장  
**파일**: `repository/IOrderRepository.h` (신규), `model/Order.h`, `repository/OrderRepository.h/.cpp`

#### 변경 내용

**`IOrderRepository.h`** — 신규 인터페이스
```cpp
class IOrderRepository : public IRepository<Order> {
public:
    virtual std::vector<Order> findByStatus(OrderStatus status) const = 0;
};
```

**`model/Order.h`** — `productionQty` 필드 추가
```cpp
int productionQty = 0;  // 실 생산량 (생산 시작 시 기록, ProductionLine 재구성에 사용)
```

**`OrderRepository`** — `IOrderRepository` 상속 변경, `findByStatus` override 표시, JSON 필드 추가
```cpp
class OrderRepository : public IOrderRepository { ... };
// productionQty JSON 직렬화/역직렬화 추가
```

**커밋**
```
feat: IOrderRepository 인터페이스 추가 및 Order productionQty 필드 추가
```

---

### Step 1 — `ProductionLine` 도메인 모델 (TDD)

**목표**: FIFO 생산 큐 구현  
**파일**: `model/ProductionLine.h`, `model/ProductionLine.cpp`  
**테스트**: `test/ProductionLineTest.cpp`

#### 도메인 모델

```cpp
struct ProductionJob {
    std::string orderNo;
    std::string sampleId;
    int         productionQty;
    long long   totalTimeSeconds;
    time_t      startTime;
};

class ProductionLine {
public:
    void enqueue(const ProductionJob& job);
    bool isEmpty() const;
    const ProductionJob& current() const;   // 빈 상태 → std::runtime_error
    bool isCurrentComplete() const;         // time(nullptr) >= startTime + totalTimeSeconds
    void completeAndAdvance();              // O(1): deque::pop_front()
    size_t size() const;
    const std::deque<ProductionJob>& jobs() const;
private:
    std::deque<ProductionJob> jobs_;        // vector 대신 deque로 O(1) front 삭제
};
```

#### 🔴 RED
```
test: ProductionLine 단위 테스트 작성
```
작성할 테스트:
- `isEmpty()` / `size()` — 생성 직후 empty
- `enqueue()` → `size()` 증가
- `current()` — 빈 상태에서 `std::runtime_error`
- `current()` — FIFO 순서 (처음 넣은 것 반환)
- `isCurrentComplete()` — 미완료: false / 이미완료: true
- `completeAndAdvance()` → 첫 번째 제거, 다음 항목으로 전진
- `completeAndAdvance()` — 마지막 항목 제거 → isEmpty
- `jobs()` — 전체 잡 목록 반환

#### 🟢 GREEN
```
feat: ProductionLine 도메인 모델 구현
```

---

### Step 2 — View 계층

**목표**: 화면 출력 + 사용자 입력 처리  
**설계 원칙**: 모든 View는 **static 메서드** + `std::istream&` / `std::ostream&` 인자 → 테스트 가능

#### MenuView
```cpp
static void printMainMenu(std::ostream& out);
static int  getMenuChoice(std::istream& in, std::ostream& out);
static void printInvalidChoice(std::ostream& out);
```

#### SampleView
```cpp
static void        printSubMenu(std::ostream& out);
static int         getSubMenuChoice(std::istream& in, std::ostream& out);
static void        printList(const std::vector<Sample>&, std::ostream& out);
static Sample      inputNewSample(std::istream& in, std::ostream& out);
static std::string inputSearchName(std::istream& in, std::ostream& out);
static void        printNotFound(std::ostream& out);
static void        printRegistered(const Sample&, std::ostream& out);
```

#### OrderView
```cpp
static Order       inputNewOrder(std::istream& in, std::ostream& out);
static void        printOrderPlaced(const Order&, std::ostream& out);
static void        printOrderList(const std::vector<Order>&, std::ostream& out); // RESERVED/CONFIRMED 공용
static std::string selectOrderNo(std::istream& in, std::ostream& out);
static bool        askApprove(std::istream& in, std::ostream& out);
static void        printApproved(const Order&, std::ostream& out);
static void        printRejected(const Order&, std::ostream& out);
static void        printInvalidOrderNo(std::ostream& out);
static void        printNoReservedOrders(std::ostream& out);
```

#### MonitorView (StockStatus는 `model/StockStatus.h`에 정의)
```cpp
// model/StockStatus.h
struct StockStatus {
    std::string sampleId, sampleName, status; // "여유"|"부족"|"고갈"
    int stock, pendingQty;
};

// view/MonitorView.h
static void printOrderSummary(int reserved, int producing, int confirmed, int release, std::ostream& out);
static void printStockStatus(const std::vector<StockStatus>&, std::ostream& out);
```

#### ProductionView
```cpp
static void printProductionLine(const ProductionLine&, std::ostream& out);
static void printEmpty(std::ostream& out);
static void printCompleted(const std::string& orderNo, std::ostream& out);
```

**커밋**
```
feat: View 계층 구현 (MenuView, SampleView, OrderView, MonitorView, ProductionView)
```

---

### Step 3 — Controller 계층 (TDD)

**목표**: 비즈니스 흐름 제어  
**Mock 헤더**: `test/MockRepositories.h`

```cpp
class MockSampleRepository : public IRepository<Sample> {
    MOCK_METHOD(std::vector<Sample>,   findAll,  (), (const, override));
    MOCK_METHOD(std::optional<Sample>, findById, (const std::string&), (const, override));
    MOCK_METHOD(void, save,   (const Sample&), (override));
    MOCK_METHOD(void, update, (const Sample&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};
class MockOrderRepository : public IOrderRepository {
    // findAll, findById, save, update, remove, findByStatus
};
```

---

#### SampleController (메뉴 1, 4)

```cpp
SampleController(IRepository<Sample>& sampleRepo, std::istream& in, std::ostream& out);
void manageSamples();                          // 서브메뉴 루프
void showMonitoring(IOrderRepository& orderRepo);
```

승인 로직 (재고 상태 판단):
```
stock == 0               → "고갈"
pending > stock, stock>0 → "부족"
pending <= stock, stock>0 → "여유"
```
> REJECTED 주문은 pending 집계에서 제외

🔴 `test: SampleController 단위 테스트 작성`  
🟢 `feat: SampleController 구현`

---

#### OrderController (메뉴 2, 3)

```cpp
OrderController(IRepository<Sample>& sampleRepo, IOrderRepository& orderRepo,
                ProductionLine& line, std::istream& in, std::ostream& out);
void placeOrder();       // 주문번호 자동 생성 (ORD-YYYYMMDD-NNNN), RESERVED 저장
void processApproval();  // RESERVED → 승인/거절 처리
```

승인 로직:
```
재고 충분 (stock >= qty):
  order.approve(true)  → CONFIRMED
  sample.stock -= qty
  sampleRepo.update()

재고 부족 (stock < qty):
  shortage = qty - stock
  prodQty  = calcProductionQty(shortage, yield)
  totalSec = calcTotalTimeSeconds(avgTime, prodQty)
  startTime = line.isEmpty() ? now : lastJob.startTime + lastJob.totalTimeSeconds
  order.productionQty = prodQty
  order.productionStartTime = startTime
  order.approve(false) → PRODUCING
  line.enqueue(job)
```

🔴 `test: OrderController 단위 테스트 작성`  
🟢 `feat: OrderController 구현`

---

#### ProductionController (메뉴 5)

```cpp
ProductionController(IRepository<Sample>& sampleRepo, IOrderRepository& orderRepo,
                     ProductionLine& line, std::ostream& out);
void showProductionLine();
```

완료 처리 로직:
```
while !line.isEmpty() && line.isCurrentComplete():
  order.completeProduction()       // PRODUCING → CONFIRMED
  sample.stock += productionQty - quantity   // 잔여 재고 반영
  orderRepo.update(), sampleRepo.update()
  line.completeAndAdvance()
```

🔴 `test: ProductionController 단위 테스트 작성`  
🟢 `feat: ProductionController 구현`

---

#### ReleaseController (메뉴 6)

```cpp
ReleaseController(IOrderRepository& orderRepo, std::istream& in, std::ostream& out);
void processRelease();
```

출고 로직:
```
CONFIRMED 목록 출력
주문번호 선택 → order.release() → RELEASE
order.releasedAt = time(nullptr)
orderRepo.update()
```

🔴 `test: ReleaseController 단위 테스트 작성`  
🟢 `feat: ReleaseController 구현`

---

### Step 4 — `main.cpp`

**목표**: 의존성 조립 + 메인 루프

```cpp
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    SampleRepository sampleRepo;
    OrderRepository  orderRepo;
    ProductionLine   line = buildProductionLine(orderRepo, sampleRepo);

    // 의존성 조립
    SampleController    sampleCtrl(sampleRepo, std::cin, std::cout);
    OrderController     orderCtrl(sampleRepo, orderRepo, line, std::cin, std::cout);
    ProductionController prodCtrl(sampleRepo, orderRepo, line, std::cout);
    ReleaseController   releaseCtrl(orderRepo, std::cin, std::cout);

    // 메인 루프
    while (true) { ... }
}
```

**ProductionLine 재구성** (앱 재시작 시):
```
PRODUCING 주문을 productionStartTime 오름차순으로 정렬
각 주문마다 ProductionJob 생성 후 line.enqueue()
```

**커밋**
```
feat: main.cpp 구현 — ProductionLine 재구성 + 메인 루프
```

---

### Step 5 — vcxproj 등록

**목표**: 신규 파일 전체를 Visual Studio 프로젝트에 등록

- `ClInclude`: 모든 신규 `.h` 파일
- `ClCompile`: 모든 신규 `.cpp` 파일
- 테스트 파일: `ExcludedFromBuild Condition="'$(Configuration)'=='Release'"`
- `main.cpp`: `ExcludedFromBuild Condition="'$(Configuration)'=='Debug'"`

**커밋**
```
chore: Phase 2 신규 파일 vcxproj 등록
```

---

## 의존 방향 (엄격 준수)

```
View → Controller → Repository → Model
```

- Controller는 View 메서드를 호출하여 출력 (Controller → View 허용)
- View는 Model 타입을 인자로 받을 수 있음 (View → Model 허용)
- `StockStatus`는 `model/StockStatus.h`에 정의 → 레이어 중립

---

## 커밋 순서 요약

| 순서 | 단계 | 커밋 메시지 |
|------|------|------------|
| 1 | Step 0 | `feat: IOrderRepository 인터페이스 추가 및 Order productionQty 필드 추가` |
| 2 | Step 1 RED | `test: ProductionLine 단위 테스트 작성` |
| 3 | Step 1 GREEN | `feat: ProductionLine 도메인 모델 구현` |
| 4 | Step 2 | `feat: View 계층 구현 (MenuView, SampleView, OrderView, MonitorView, ProductionView)` |
| 5 | Step 3 RED | `test: Controller 단위 테스트 작성 (SampleController, OrderController, ProductionController, ReleaseController)` |
| 6 | Step 3 GREEN | `feat: Controller 계층 구현 (SampleController, OrderController, ProductionController, ReleaseController)` |
| 7 | Step 4 | `feat: main.cpp 구현 — ProductionLine 재구성 + 메인 루프` |
| 8 | Step 5 | `chore: Phase 2 신규 파일 vcxproj 등록` |

---

## PR 정보

- **제목**: `[Phase 2] Controller + View 계층 구현 — 콘솔 애플리케이션 완성`
- **base**: `main`
- **compare**: `phase/2-controller-view`
