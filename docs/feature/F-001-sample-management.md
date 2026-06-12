# F-001 시료 관리 (Sample Management)

## 개요
생산담당자가 시료를 등록하고 목록 조회·이름 검색을 수행하는 기능.  
메인 메뉴 **[1] 시료 관리** 에 해당한다.

---

## 관련 파일

| 레이어 | 파일 |
|--------|------|
| Model | `model/Sample.h`, `model/Sample.cpp` |
| Repository | `repository/IRepository.h`, `repository/SampleRepository.h`, `repository/SampleRepository.cpp` |
| Controller | `controller/SampleController.h`, `controller/SampleController.cpp` |
| View | `view/SampleView.h`, `view/SampleView.cpp` |
| DB | `data/samples.json` |

---

## 기능 상세

### F-001-1 시료 등록

**흐름**
1. 시료 ID 입력 (`S-NNN` 형식, 예: `S-001`)
2. 시료명 입력
3. 평균 생산시간 입력 (min/ea, 양의 정수)
4. 수율 입력 (0.0 초과 ~ 1.0 이하, double)
5. 초기 재고 수량 입력 (0 이상 정수)
6. 중복 ID 검사 → 중복 시 오류 출력 후 재입력 요청
7. `SampleRepository::save()` 호출 → `data/samples.json` 즉시 저장
8. 등록 완료 메시지 출력 (초록)

**입력 유효성 검사**

| 필드 | 규칙 |
|------|------|
| ID | `S-` 로 시작, 이후 숫자 1자리 이상, 중복 불가 |
| 평균 생산시간 | 1 이상 정수 |
| 수율 | 0.0 < yield ≤ 1.0 |
| 재고 | 0 이상 정수 |

### F-001-2 시료 목록 조회

- `SampleRepository::findAll()` 로 전체 시료 로드
- ID / 이름 / 평균 생산시간 / 수율 / 현재 재고 컬럼 테이블 출력
- 등록된 시료 없을 시 안내 메시지 출력

### F-001-3 시료 이름 검색

- 키워드 입력 후 `name.find(keyword) != npos` 부분 일치 검색
- 결과 목록 테이블 출력 (F-001-2 동일 포맷)
- 결과 없을 시 안내 메시지 출력 (노랑)

---

## 데이터 모델

```cpp
struct Sample {
    std::string id;               // "S-001"
    std::string name;             // "실리콘 웨이퍼-8인치"
    int         avgProductionTime; // min/ea
    double      yield;            // 0.0 < yield <= 1.0
    int         stock;            // 현재 보유 수량
};
```

---

## 예외 처리

| 상황 | 처리 |
|------|------|
| 중복 ID | 오류 메시지(빨강) 후 재입력 |
| 수율 범위 초과 | 오류 메시지(빨강) 후 재입력 |
| 빈 입력 | 오류 메시지(빨강) 후 재입력 |
| JSON 파일 쓰기 실패 | 오류 메시지 출력 후 등록 취소 |

---

## 테스트 케이스

| ID | 시나리오 | 기대 결과 |
|----|----------|----------|
| TC-001-01 | 정상 시료 등록 | samples.json 저장, 완료 메시지 |
| TC-001-02 | 중복 ID 등록 | 오류 메시지, 재입력 요청 |
| TC-001-03 | 수율 1.0 초과 입력 | 오류 메시지, 재입력 요청 |
| TC-001-04 | 수율 0.0 입력 | 오류 메시지, 재입력 요청 |
| TC-001-05 | 전체 목록 조회 (시료 0개) | 안내 메시지 출력 |
| TC-001-06 | 전체 목록 조회 (시료 N개) | N행 테이블 출력 |
| TC-001-07 | 이름 검색 — 부분 일치 존재 | 일치 목록 출력 |
| TC-001-08 | 이름 검색 — 일치 없음 | 안내 메시지 출력 |
