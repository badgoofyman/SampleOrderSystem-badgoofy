# F-009 데이터 영속성 (Data Persistence)

## 개요
시료(Sample)와 주문(Order) 데이터를 JSON 파일로 저장·로드하는 Repository 구현.  
애플리케이션 재실행 후에도 마지막 저장 상태를 완전히 복원한다.

---

## 관련 파일

| 레이어 | 파일 |
|--------|------|
| Interface | `repository/IRepository.h` |
| Repository | `repository/SampleRepository.h`, `repository/SampleRepository.cpp` |
| Repository | `repository/OrderRepository.h`, `repository/OrderRepository.cpp` |
| DB | `data/samples.json`, `data/orders.json` |

---

## 저장소 인터페이스

```cpp
template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;

    virtual std::vector<T>  findAll()                          const = 0;
    virtual std::optional<T> findById(const std::string& id)  const = 0;
    virtual void             save(const T& entity)                   = 0;
    virtual void             update(const T& entity)                 = 0;
    virtual void             remove(const std::string& id)           = 0;
};
```

- Controller는 구체 클래스가 아닌 `IRepository<T>` 인터페이스에만 의존한다 (DI).
- Mock 테스트 시 `MockSampleRepository`, `MockOrderRepository`로 교체 가능.

---

## JSON 파일 경로

| 데이터 | 파일 경로 |
|--------|----------|
| 시료 | `data/samples.json` |
| 주문 | `data/orders.json` |

- 실행 파일 기준 상대 경로 `data/` 를 사용한다.
- 파일이 없을 경우 빈 배열(`[]`)로 초기화하여 생성한다.

---

## samples.json 스키마

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

## orders.json 스키마

```json
[
  {
    "orderNo": "ORD-20260612-0001",
    "sampleId": "S-001",
    "customerName": "삼성전자",
    "quantity": 20,
    "status": "PRODUCING",
    "productionStartTime": 1749689400
  }
]
```

- `status` 값: `"RESERVED"` | `"PRODUCING"` | `"CONFIRMED"` | `"RELEASE"` | `"REJECTED"`
- `productionStartTime`: PRODUCING 상태에서만 유효한 UTC epoch(초). 그 외 상태는 `0`.

---

## 영속성 규칙

| 규칙 | 설명 |
|------|------|
| 즉시 쓰기 | 모든 데이터 변경 후 즉시 JSON 파일 전체를 재기록한다 |
| 시작 시 로드 | 애플리케이션 시작 시 JSON 파일 전체를 메모리로 로드한다 |
| 파일 없음 처리 | 파일 미존재 시 빈 배열로 초기화 후 생성 |
| 파싱 오류 처리 | JSON 파싱 실패 시 오류 메시지 출력 후 프로그램 종료 |

---

## 직렬화 / 역직렬화

JSON 파싱은 **nlohmann/json** 헤더 전용 라이브러리를 사용한다.

```cpp
// 직렬화 (Sample → JSON)
nlohmann::json toJson(const Sample& s);

// 역직렬화 (JSON → Sample)
Sample fromJson(const nlohmann::json& j);
```

---

## 예외 처리

| 상황 | 처리 |
|------|------|
| 파일 읽기 실패 | 빈 컬렉션으로 초기화 |
| JSON 파싱 오류 | 오류 메시지(빨강) 출력 후 프로그램 종료 |
| 파일 쓰기 실패 | 오류 메시지(빨강) 출력, 호출자에 예외 전파 |

---

## 테스트 케이스

| ID | 시나리오 | 기대 결과 |
|----|----------|----------|
| TC-009-01 | Sample 저장 후 findAll | 저장한 데이터 반환 |
| TC-009-02 | Order 저장 후 findById | 저장한 주문 반환 |
| TC-009-03 | Order update (status 변경) | 변경된 status 반환 |
| TC-009-04 | productionStartTime 저장·로드 | epoch 값 정확히 복원 |
| TC-009-05 | 파일 없을 때 findAll | 빈 벡터 반환, 파일 생성 |
| TC-009-06 | 저장 후 프로그램 재실행, findAll | 이전 데이터 그대로 반환 |
| TC-009-07 | remove 후 findById | `std::nullopt` 반환 |
