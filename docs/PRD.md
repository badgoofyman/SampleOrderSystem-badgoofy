# PRD — 반도체 시료 생산주문관리 시스템 (S-Semi SampleOrderSystem)

**작성일**: 2026-06-12  
**버전**: 1.0  
**Repository**: `SampleOrderSystem-badgoofy`

---

## 1. 제품 개요

### 1.1 배경

가상의 반도체 회사 **S-Semi**는 고객의 시료(Sample) 요청에서부터 생산, 출고까지의 전 과정을 수작업으로 관리하고 있다. 이를 디지털화하여 주문 추적, 생산 스케줄링, 재고 현황을 통합 관리하는 콘솔 기반 시스템이 필요하다.

### 1.2 목적

- 시료 주문 접수 → 승인/거절 → 생산 → 출고의 전 과정을 단일 시스템으로 관리
- 주문 상태를 State Machine으로 일관되게 추적하여 처리 오류 방지
- JSON 파일 기반 데이터 영속성으로 별도 DB 서버 없이 운영 가능
- 콘솔 컬러 출력으로 상태·경고·오류를 시각적으로 구분하여 가독성 향상

### 1.3 구현기능

- 시료 등록 및 조회
- 주문 접수·승인·거절
- 생산라인 FIFO 스케줄링
- 재고 모니터링
- 출고 처리

> **단일 독립 실행 프로그램**: 외부 서버, 외부 API, 외부 서비스와의 연동 없이 단독으로 동작한다. 데이터는 로컬 JSON 파일로만 관리한다.

---

## 2. 사용자 및 역할

| 역할 | 책임 |
|------|------|
| **주문담당자** | 고객 요청을 기반으로 주문서 작성 (메뉴 [2]) |
| **생산담당자** | 시료 등록, 주문 승인/거절, 생산라인 관리, 출고 처리 (메뉴 [1][3][5][6]) |

```
고객 → (시료 요청) → 주문담당자 → (주문서 전달) → 생산담당자
                         ↑                              |
                         └─────── (승인 / 거절) ────────┘
```

---

## 3. 기능 요구사항

### 3.1 메인 메뉴 구조

| 번호 | 메뉴명 | 담당 Controller |
|------|--------|----------------|
| [1] | 시료 관리 | `SampleController` |
| [2] | 시료 주문 | `OrderController` |
| [3] | 주문 승인/거절 | `OrderController` |
| [4] | 모니터링 | `SampleController` + `OrderController` |
| [5] | 생산라인 조회 | `ProductionController` |
| [6] | 출고 처리 | `ReleaseController` |
| [0] | 종료 | — |

**메인 화면 시스템 현황** (앱 시작 시 JSON에서 집계하여 표시)

| 항목 | 집계 방법 |
|------|----------|
| 등록 시료 수 | `samples.json` 항목 수 |
| 총 재고 | 전체 시료 `stock` 합산 |
| 전체 주문 | `orders.json` 항목 수 (REJECTED 제외) |
| 생산라인 대기 | PRODUCING 상태 주문 수 |

- 메뉴는 **2열 배치**: 홀수 번호(좌) / 짝수 번호(우)
- 상단 ASCII Art 로고(`S-Semi`) 하드코딩 출력

---

### 3.2 [1] 시료 관리

서브메뉴: `[1] 시료 등록` / `[2] 시료 목록` / `[3] 시료 검색` / `[0] 위로`

#### 3.2.1 시료 등록
- 입력 순서: 시료 ID → 시료명 → 평균 생산시간 → 수율 → 초기 재고
- 시료 ID는 `S-NNN` 형식 (예: `S-001`)
- 중복 ID 시: `오류: 이미 존재하는 시료 ID입니다.` 출력 후 재입력
- 수율 범위 위반 시: 오류 메시지 후 재입력
- 등록된 시료는 `data/samples.json`에 즉시 저장
- 등록 완료 후 입력값 요약 출력

#### 3.2.2 시료 목록 조회
- 전체 시료 목록 출력: ID / 시료명 / 평균 생산시간 / 수율 / 현재 재고
- **페이지네이션**: 한 페이지 5행 표시, `[N]` 입력 시 다음 페이지
- 재고 0인 항목은 빨간색으로 강조

#### 3.2.3 시료 이름 검색
- 키워드 부분 일치 검색 (**대소문자 무시**)
- 결과 없을 시: `검색 결과가 없습니다.` 출력

---

### 3.3 [2] 시료 주문

- 입력 순서: 시료 ID → 고객명 → 주문 수량
- 존재하지 않는 시료 ID 입력 시: `오류: 등록되지 않은 시료 ID입니다.` 후 재입력
- **입력 확인 화면**: 시료명·고객명·수량 요약 표시 후 `[Y] 예약 접수 / [N] 취소` 선택
  - N 선택 시 입력 취소, 메뉴로 복귀
- 주문번호 자동 생성: `ORD-YYYYMMDD-NNNN` 형식 (예: `ORD-20260416-0043`)
  - 날짜: 주문 접수일
  - 순번: `orders.json`에서 당일 날짜 주문 건수를 카운트하여 +1, 4자리 zero-padding
- 생성된 주문 상태: **RESERVED**
- 주문 정보는 `data/orders.json`에 즉시 저장
- 완료 후 주문번호와 상태 배지 출력, 승인 메뉴 안내 문구(`※`) 표시

---

### 3.4 [3] 주문 승인/거절

- RESERVED 주문 목록을 **번호** 형태로 표시 (번호, 주문번호, 고객, 시료, 수량, 상태)
- 목록 없을 시: `대기 중인 주문이 없습니다.` 출력 후 복귀
- 처리할 **번호** 입력으로 주문 선택
- 재고 확인 화면: 현재 재고, 주문 수량, 부족분 표시
- 승인 처리:
  - **재고 충분** (재고 ≥ 수량): `재고 충분. 즉시 출고 대기로 전환합니다.` → `[Y] 승인 / [N] 거절` 선택
    - Y: CONFIRMED 전환 + 재고 차감
  - **재고 부족** (재고 < 수량): 실생산량·총생산시간(min) 계산 결과 표시 → `[Y] 승인 / [N] 거절` 선택
    - Y: PRODUCING 전환 + `productionStartTime` 기록 (재고 차감 없음)
  - N (거절): REJECTED 전환, 재고 변경 없음
- 결과 화면: 상태 변경 (`RESERVED → [새 상태]`) 및 주문번호 출력

> **재고 차감 시점**: 재고는 항상 **CONFIRMED 전환 시점**에 차감한다.  
> PRODUCING → CONFIRMED 자동 전환 시에도 동일하게 CONFIRMED 전환 시 재고를 차감한다.

---

### 3.5 [4] 모니터링

서브메뉴: `[1] 주문량 확인` / `[2] 재고량 확인` / `[0] 뒤로`

#### 3.5.1 주문량 확인 ([1])
- RESERVED / PRODUCING / CONFIRMED / RELEASE 건수 표시
- **REJECTED는 집계에서 제외**
- PRODUCING 건수 옆 `← 생산라인 대기` 안내 문구 표시

#### 3.5.2 재고량 확인 ([2])
- 시료명 / 재고 / 상태 배지 / **프로그레스 바** / 잔여율(%) 출력

| 표기 | 조건 |
|------|------|
| **여유** | 재고 > 0 이고 활성 주문(PRODUCING+CONFIRMED) 합산 수량 ≤ 재고 |
| **부족** | 재고 > 0 이지만 활성 주문 합산 수량 > 재고 |
| **고갈** | 재고 = 0 |

**프로그레스 바**
```
[████████░░░░░░░░░░░░]  44%
```
- 전체 20칸, `█`(채움) / `░`(빈칸)
- 잔여율 = `stock / (stock + 활성주문수량합계) × 100` (활성주문 없으면 100%)
- 색상은 재고 상태와 연동 (여유: 초록, 부족: 노랑, 고갈: 빨강)

---

### 3.6 [5] 생산라인 조회

- 메뉴 진입 시 생산 완료 항목 자동 처리 재수행 (앱 시작 시 처리 이후 추가 경과 시간 반영)
- **현재 처리 중** 항목: 박스(`┌─┘`) 형태로 강조
  - 주문번호, 시료, 주문량, 재고, 부족분, 실생산량, 수율, 총생산시간
  - 진행률 프로그레스 바 (`경과시간 / 총생산시간 × 100`) + 예상 완료 시각
- **대기 큐** (FIFO 순): 순번, 주문번호, 시료, 주문량, 부족분, 실생산량, 예상 완료 시각
  - 예상 완료 시각: 앞 항목 완료 후 순차 누적 계산
- 생산라인 유휴 시 상태 `[ IDLE ]` 표시, 현재 처리 중 박스 생략
- 대기 주문 없을 시: `대기 중인 주문이 없습니다.` 출력

---

### 3.7 [6] 출고 처리

- CONFIRMED 주문 목록을 **번호** 형태로 표시 (번호, 주문번호, 고객, 시료, 수량)
- 목록 없을 시: `출고 대기 중인 주문이 없습니다.` 출력 후 복귀
- 처리할 **번호** 입력으로 주문 선택
- 출고 실행: 상태를 **RELEASE**로 전환, 처리일시(`releasedAt`) 기록
- 결과 화면: 주문번호, 출고 수량, 처리일시, 상태 변경 출력

---

## 4. 도메인 모델

### 4.1 시료 (Sample)

| 필드 | 타입 | 설명 | 예시 |
|------|------|------|------|
| `id` | `string` | 고유 식별자 | `S-001` |
| `name` | `string` | 시료명 | `실리콘 웨이퍼-8인치` |
| `avgProductionTime` | `int` | 단위당 생산 소요 시간 (min/ea) | `120` |
| `yield` | `double` | 수율 (0 < yield ≤ 1.0) | `0.92` |
| `stock` | `int` | 현재 보유 수량 | `50` |

### 4.2 주문 (Order)

| 필드 | 타입 | 설명 | 예시 |
|------|------|------|------|
| `orderNo` | `string` | 주문번호 (자동 생성) | `ORD-20260416-0043` |
| `sampleId` | `string` | 주문 대상 시료 ID | `S-001` |
| `customerName` | `string` | 주문 고객명 | `삼성전자` |
| `quantity` | `int` | 주문 수량 | `20` |
| `status` | `OrderStatus` | 주문 상태 (열거형) | `RESERVED` |
| `productionStartTime` | `time_t` | PRODUCING 전환 시각 (UTC epoch, 초). 비생산 상태에서는 `0` | `1744771200` |
| `releasedAt` | `time_t` | RELEASE 전환 시각 (UTC epoch, 초). RELEASE 이전 상태에서는 `0` | `1744858800` |

---

## 5. 주문 상태 흐름 (State Machine)

```
RESERVED → (승인) → 재고 충분 → CONFIRMED → RELEASE
                 → 재고 부족 → PRODUCING → CONFIRMED → RELEASE
         → (거절) → REJECTED
```

### 5.1 허용 전환 테이블

| From | To | 조건 |
|------|----|------|
| `RESERVED` | `CONFIRMED` | 승인 + 재고 충분 |
| `RESERVED` | `PRODUCING` | 승인 + 재고 부족 |
| `RESERVED` | `REJECTED` | 거절 |
| `PRODUCING` | `CONFIRMED` | 생산 완료 (자동) |
| `CONFIRMED` | `RELEASE` | 출고 처리 |

- 위 표에 없는 전환은 모두 불법 전환으로 예외 처리

---

## 6. 생산라인 계산 공식

```
부족분         = 주문 수량 - 현재 재고 할당량
실 생산량      = ceil(부족분 / (수율 × 0.9))
총 생산시간    = 평균 생산시간(min/ea) × 실 생산량   [단위: 분]
```

- 수율과 안전 계수 `0.9`는 **생산 투입량(실 생산량) 산정에만** 사용한다.
- **실제 생산 수율은 100%**: 생산 완료 후 실 생산량 전량을 양품으로 취급하여 재고에 반영한다.
- 계산 로직은 `util/ProductionCalculator`에 격리
- 스케줄링: **FIFO** (단일 라인, 선입선출)

### 6.1 실제 시간 기반 생산 처리

생산은 **실제 벽시계 시간(wall-clock time)** 기준으로 진행된다.

- 주문이 PRODUCING 상태로 전환될 때 `productionStartTime`(UTC epoch, 초 단위)을 기록하여 `orders.json`에 저장한다.
- **프로그램을 재실행해도 생산은 계속 진행**된다.

```
완료 조건: (현재 시각 - productionStartTime) ≥ 총 생산시간(분) × 60
```

**생산 완료 자동 처리 시점: 앱 시작 시 즉시**

```
앱 시작
  └─ PRODUCING 주문 전체 조회
       └─ 완료 조건 충족 여부 평가 (FIFO 순)
            ├─ 충족: stock += 실생산량, stock -= quantity → CONFIRMED 전환 → orders/samples.json 저장
            └─ 미충족: 유지
  └─ 메인 화면 표시
```

- 재실행 즉시 경과 시간을 재계산하여, 생산이 완료된 주문은 메인 화면 진입 전에 CONFIRMED로 전환된다.
- `[5] 생산라인 조회` 메뉴 진입 시에도 동일한 완료 처리를 재수행한다 (조회 중 시간 경과 반영).

### 6.2 재고 선순위 할당 (FIFO)

현재 보유 재고는 **주문 접수 순서(RESERVED 등록 순)** 로 먼저 할당한다.

- 동일 시료에 대해 복수의 RESERVED 주문이 있을 경우, 먼저 접수된 주문부터 재고를 우선 배정한다.
- 재고로 충족 가능한 주문은 CONFIRMED, 나머지는 PRODUCING으로 전환된다.
- 예: 재고 15ea, 주문A 10ea(선접수) + 주문B 10ea(후접수) → 주문A CONFIRMED(재고 10차감), 주문B PRODUCING(부족분 10ea 생산)

### 6.3 계산 예시

| 항목 | 값 |
|------|----|
| 주문 수량 | 30 ea |
| 현재 재고 할당량 | 10 ea |
| 부족분 | 20 ea |
| 수율 | 0.92 |
| 실 생산량 | `ceil(20 / (0.92 × 0.9))` = `ceil(20 / 0.828)` = `ceil(24.15)` = **25 ea** |
| 평균 생산시간 | 120 min/ea |
| 총 생산시간 | 120 × 25 = **3,000 min** |
| 재고 증가량 | **25 ea** (실 생산량 전량 양품) |

---

## 7. 데이터 영속성

### 7.1 파일 구조

```
data/
├── samples.json    # 시료 DB
└── orders.json     # 주문 DB
```

### 7.2 samples.json 스키마

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

### 7.3 orders.json 스키마

```json
[
  {
    "orderNo": "ORD-20260416-0043",
    "sampleId": "S-001",
    "customerName": "삼성전자",
    "quantity": 20,
    "status": "PRODUCING",
    "productionStartTime": 1744771200,
    "releasedAt": 0
  }
]
```

- `productionStartTime`: PRODUCING 전환 시 기록하는 UTC epoch(초). 그 외 상태에서는 `0`.
- `releasedAt`: RELEASE 전환 시 기록하는 UTC epoch(초). RELEASE 이전 상태에서는 `0`.
- 재실행 후에도 `productionStartTime`으로 경과 시간을 계산하여 생산 완료 여부를 판단한다.

### 7.4 영속성 규칙

- 모든 데이터 변경 시 즉시 JSON 파일에 반영 (쓰기 후 검증)
- 애플리케이션 시작 시 JSON 파일에서 데이터 로드
- 파일 없을 시 빈 배열로 초기화

---

## 8. 아키텍처 및 설계 원칙

### 8.1 MVC + Repository 레이어

```
View → Controller → Repository → Model
```

| 레이어 | 책임 |
|--------|------|
| **View** | 화면 출력, 사용자 입력 수집만 담당 |
| **Controller** | 비즈니스 흐름 제어, View와 Repository 연결 |
| **Repository** | JSON 파일 CRUD, 도메인 객체 변환 |
| **Model** | 순수 데이터 구조 + 도메인 비즈니스 로직 |

- View → Repository 직접 접근 **금지**
- View → Model 직접 접근 **금지**
- Controller → JSON 파일 직접 접근 **금지**

### 8.2 인터페이스 설계

- `IRepository<T>`: Mock 테스트를 위한 저장소 추상화
- Controller는 `IRepository` 인터페이스에만 의존 (DI 가능)

---

## 9. 기술 스택

| 항목 | 내용 |
|------|------|
| 언어 | C++20 |
| IDE | Visual Studio 2022 (툴셋 v145) |
| 솔루션 파일 | `SampleOrderSystem.slnx` |
| 플랫폼 | x64 / Win32, Debug / Release |
| 테스트 프레임워크 | gmock 1.11.0 (NuGet) |
| DB | JSON 파일 (별도 DB 서버 없음) |
| JSON 파싱 | 자체 구현 또는 헤더 전용 라이브러리 |
| 소스 파일 인코딩 | **UTF-8 with BOM** (서명 있는 UTF-8): 한글이 포함된 모든 `.h` / `.cpp` 파일에 적용 |

---

## 10. 테스트 요구사항

### 10.1 Debug 빌드 시 단위 테스트 자동 실행

**Debug 빌드 완료 후 단위 테스트가 자동으로 실행**된다.

- Visual Studio의 **빌드 후 이벤트(Post-Build Event)** 를 사용하여 Debug 구성에서만 테스트 실행
- Release 빌드에서는 테스트 실행하지 않음
- 테스트 실패 시 빌드 결과에 오류로 표시

```xml
<!-- .vcxproj Post-Build Event (Debug 구성만 적용) -->
<PostBuildEvent Condition="'$(Configuration)'=='Debug'">
  <Command>$(TargetPath)</Command>
</PostBuildEvent>
```

> 테스트 진입점은 `gmock_main`(`gmock.1.11.0`)이 제공하는 `main()`을 사용하므로  
> 별도의 `main()` 없이 테스트 파일만 프로젝트에 포함하면 자동 실행된다.

### 10.2 단위 테스트 필수 항목

| 대상 | 테스트 내용 |
|------|------------|
| `ProductionCalculator` | 다양한 수율/부족분 조합의 실 생산량 계산 정확성 |
| `Order` State Machine | 합법 전환 (5가지) 정상 동작 확인 |
| `Order` State Machine | 불법 전환 시 예외 발생 확인 |
| `SampleController` | Mock Repository 주입 후 시료 등록/조회 흐름 |
| `OrderController` | Mock Repository 주입 후 승인/거절 흐름 |
| `ProductionController` | FIFO 큐 순서 및 생산 완료 자동 상태 전환 |

### 10.3 테스트 작성 규칙

- 테스트 파일 위치: `test/` 디렉토리
- gmock 헤더: `#include <gmock/gmock.h>`
- Controller 테스트는 반드시 `IRepository` Mock 주입으로 격리
- 테스트 함수명: `TEST(대상클래스, 테스트시나리오)` 형식
- 테스트 파일도 한글 포함 시 **UTF-8 with BOM** 인코딩 적용

---

## 11. 비기능 요구사항

| 항목 | 요구사항 |
|------|---------|
| 응답성 | 메뉴 선택 후 1초 이내 화면 전환 |
| 데이터 안전성 | 비정상 종료 후 재실행 시 마지막 저장 상태 복원 |
| 입력 검증 | 잘못된 메뉴 번호, 빈 입력, 범위 초과 수량 모두 오류 처리 |
| 코드 품질 | CleanCode 원칙: 단일 책임 함수, 명확한 네이밍, 주석 최소화 |
| 빌드 | Visual Studio 2022에서 경고 없이 빌드 성공 |
| Debug 빌드 | 빌드 완료 후 단위 테스트 자동 실행 (Post-Build Event) |
| 소스 인코딩 | 한글 포함 파일은 **UTF-8 with BOM** (서명 있는 UTF-8) 저장 |
| UI | 콘솔 컬러 출력으로 정보 유형을 시각적으로 구분 |

### 11.1 콘솔 컬러 표준

Windows ANSI 이스케이프 코드(`\033[...m`)를 사용한다. 앱 시작 시 `SetConsoleMode`로 `ENABLE_VIRTUAL_TERMINAL_PROCESSING` 활성화.

**일반 컬러**

| 색상 | ANSI 코드 | 용도 |
|------|----------|------|
| 흰색(White) | `\033[97m` | 일반 텍스트, 입력 프롬프트 |
| 하늘색(Cyan) | `\033[96m` | 메뉴 번호 `[N]` |
| 밝은 파란색(Bright Blue) | `\033[94m` | 테이블 헤더 |
| 노란색(Yellow) | `\033[93m` | 강조 수치(총 재고·대기 건수), 안내/경고 `※` |
| 초록색(Green) | `\033[92m` | 완료 메시지 |

**상태 배지 컬러**

| 상태 | 색상 | ANSI 코드 |
|------|------|----------|
| `RESERVED` | 파란색 | `\033[94m` |
| `PRODUCING` | 주황색 | `\033[33m` |
| `CONFIRMED` | 초록색 | `\033[92m` |
| `RELEASE` | 보라색 | `\033[95m` |
| `RUNNING` | 초록색 | `\033[92m` |
| `IDLE` | 흰색 | `\033[97m` |

**재고 상태 배지 컬러**

| 상태 | 색상 | ANSI 코드 |
|------|------|----------|
| `여유` | 초록색 | `\033[92m` |
| `부족` | 노란색 | `\033[93m` |
| `고갈` | 빨간색 | `\033[91m` |

### 11.2 컬러 출력 구현 원칙

- 컬러 출력 로직은 `util/ConsoleColor.h`에 유틸 함수로 격리
- View 레이어에서만 컬러 함수 호출 (Controller·Model에서 직접 출력 금지)
- 컬러 리셋(`\033[0m`)은 출력 직후 반드시 수행하여 다음 출력에 영향 없도록 처리

---

## 12. 폴더 구조

```
SampleOrderSystem/
├── model/
│   ├── Sample.h / Sample.cpp
│   ├── Order.h / Order.cpp
│   └── ProductionLine.h / ProductionLine.cpp
├── repository/
│   ├── IRepository.h
│   ├── SampleRepository.h / SampleRepository.cpp
│   └── OrderRepository.h / OrderRepository.cpp
├── controller/
│   ├── SampleController.h / SampleController.cpp
│   ├── OrderController.h / OrderController.cpp
│   ├── ProductionController.h / ProductionController.cpp
│   └── ReleaseController.h / ReleaseController.cpp
├── view/
│   ├── MenuView.h / MenuView.cpp
│   ├── SampleView.h / SampleView.cpp
│   ├── OrderView.h / OrderView.cpp
│   ├── MonitorView.h / MonitorView.cpp
│   └── ProductionView.h / ProductionView.cpp
├── util/
│   ├── ProductionCalculator.h / ProductionCalculator.cpp
│   └── ConsoleColor.h
├── test/
│   └── *.cpp
├── data/
│   ├── samples.json
│   └── orders.json
├── packages/
│   └── gmock.1.11.0/
├── main.cpp
├── CLAUDE.md
└── PRD.md
```

---

## 13. 개발 방법론 — Agentic Engineering

### 13.1 원칙

- 각 Phase는 **독립적으로 완결**되어야 한다: 해당 Phase의 기능이 E2E로 동작한 것을 확인한 후 다음 Phase로 진행한다.
- Phase 내 구현 순서: **단위 테스트 작성 → 구현 → 단위 테스트 통과 → E2E 시나리오 확인**
- 각 Phase 완료 기준: 아래 E2E 체크리스트를 모두 통과

---

### 13.2 Git 워크플로우

#### 브랜치 전략

```
main
└── phase/1-foundation
└── phase/2-sample-ui
└── phase/3-order
└── phase/4-production-release
└── phase/5-monitoring-integration
```

- `main` 브랜치는 **항상 동작 가능한 상태**를 유지한다.
- 각 Phase 시작 시 `main`에서 브랜치를 생성한다.
- Phase 완료 후 PR을 통해서만 `main`에 병합한다.

#### Phase별 브랜치 명 규칙

| Phase | 브랜치명 |
|-------|---------|
| Phase 1 | `phase/1-foundation` |
| Phase 2 | `phase/2-sample-ui` |
| Phase 3 | `phase/3-order` |
| Phase 4 | `phase/4-production-release` |
| Phase 5 | `phase/5-monitoring-integration` |

#### TDD 단계별 커밋 규칙

각 기능 단위를 아래 3단계로 나누어 커밋한다.

```
[RED]   test: 실패하는 단위 테스트 작성
[GREEN] feat: 테스트를 통과하는 최소 구현
[BLUE]  refactor: 코드 정리 (테스트 통과 유지)
```

**커밋 메시지 접두사**

| 접두사 | 용도 |
|--------|------|
| `test:` | 단위 테스트 추가·수정 |
| `feat:` | 기능 구현 |
| `refactor:` | 리팩터링 (동작 변경 없음) |
| `fix:` | 버그 수정 |
| `docs:` | 문서 수정 |
| `chore:` | 빌드 설정·기타 |

**Phase 내 커밋 흐름 예시 (Phase 1)**

```
test: Order State Machine 합법·불법 전환 테스트 작성
feat: Order State Machine 구현
test: ProductionCalculator 수치 계산 테스트 작성
feat: ProductionCalculator 구현
test: SampleRepository JSON CRUD 테스트 작성
feat: SampleRepository 구현
test: OrderRepository JSON CRUD 테스트 작성
feat: OrderRepository 구현
```

#### PR 발행 규칙

- **PR 제목**: `[Phase N] 단계명` (예: `[Phase 1] 기반 인프라 — 도메인 모델 + Repository`)
- **PR 본문**: E2E 체크리스트 전체 포함, 통과 항목 체크
- **병합 조건**:
  - Debug 빌드 단위 테스트 전체 통과
  - E2E 체크리스트 전 항목 통과
  - 리뷰어 승인 (셀프 리뷰 포함)
- **병합 방식**: Squash merge 또는 Merge commit (히스토리 보존 목적)

### 13.3 Phase별 개발 계획

---

#### Phase 1 — 기반 인프라

**구현 대상**
- `model/Sample`, `model/Order` (도메인 모델 + State Machine)
- `repository/IRepository`, `repository/SampleRepository`, `repository/OrderRepository`
- `util/ProductionCalculator`
- `data/samples.json`, `data/orders.json` 초기화

**단위 테스트**
- Order State Machine 합법·불법 전환 전체
- ProductionCalculator 수치 계산
- SampleRepository / OrderRepository JSON CRUD (저장·로드·수정·삭제)

**E2E 체크리스트**
- [ ] `samples.json` / `orders.json` 없는 상태에서 앱 시작 시 빈 파일 자동 생성
- [ ] Sample 객체를 생성하여 저장 후 재실행 시 동일 데이터 복원
- [ ] Order 상태 전환 5가지 정상 동작, 불법 전환 예외 발생 확인
- [ ] `ceil(부족분 / (수율 × 0.9))` 계산 결과 수치 검증

---

#### Phase 2 — 시료 관리 + 콘솔 UI 기반

**구현 대상**
- `util/ConsoleColor`
- `view/MenuView`, `view/SampleView`
- `controller/SampleController`
- `main.cpp` (메인 루프 + ANSI 컬러 활성화)

**단위 테스트**
- SampleController Mock Repository 주입 후 등록·조회·검색 흐름

**E2E 체크리스트**
- [ ] 메인 화면 ASCII Art 로고 및 시스템 현황 출력 확인
- [ ] `[1]` 시료 등록 → 저장 → 재실행 후 목록 조회에서 확인
- [ ] 중복 ID 등록 시 오류 메시지 후 재입력 요청 동작
- [ ] 수율 범위 오류 처리 동작
- [ ] 목록 페이지네이션 (5행/페이지, `[N]` 다음 페이지)
- [ ] 이름 검색 부분 일치·대소문자 무시 동작

---

#### Phase 3 — 주문 접수 + 승인/거절

**구현 대상**
- `view/OrderView`
- `controller/OrderController`

**단위 테스트**
- OrderController Mock Repository 주입 후 승인·거절 흐름
- FIFO 재고 선순위 할당 로직

**E2E 체크리스트**
- [ ] `[2]` 주문 접수 → 확인 화면(Y/N) → Y 선택 시 RESERVED 저장
- [ ] 주문번호 `ORD-YYYYMMDD-NNNN` 형식 및 당일 순번 정상 생성
- [ ] `[3]` RESERVED 목록 표시 → 번호 선택 → 재고 확인 화면 출력
- [ ] 재고 충분 → 승인 → CONFIRMED 전환 + 재고 차감 확인
- [ ] 재고 부족 → 승인 → PRODUCING 전환 + `productionStartTime` 기록 확인
- [ ] 거절 → REJECTED 전환, 재고 불변 확인
- [ ] FIFO 재고 할당: 선접수 주문 우선 재고 배정 확인

---

#### Phase 4 — 생산라인 + 출고 처리

**구현 대상**
- `model/ProductionLine`
- `view/ProductionView`
- `controller/ProductionController`, `controller/ReleaseController`
- 앱 시작 시 생산 완료 자동 처리 (`main.cpp`)

**단위 테스트**
- ProductionController FIFO 큐 정렬 및 완료 자동 전환
- 앱 시작 시 완료 조건 평가 로직

**E2E 체크리스트**
- [ ] `[5]` 생산라인 조회: 현재 처리 중 박스 + 진행률 프로그레스 바 출력
- [ ] 총 생산시간 경과 후 `[5]` 진입 시 CONFIRMED 자동 전환 확인
- [ ] **앱 재실행 시** 생산 완료 조건 충족 주문이 메인 화면 진입 전 CONFIRMED 전환 확인
- [ ] 생산라인 유휴 시 `[ IDLE ]` 상태 표시
- [ ] `[6]` 출고 처리: CONFIRMED 목록 → 번호 선택 → RELEASE 전환 + 처리일시 기록
- [ ] `[6]` 완료 후 `[4]` 모니터링에서 RELEASE 건수 증가 확인

---

#### Phase 5 — 모니터링 + 통합 검증

**구현 대상**
- `view/MonitorView`
- 모니터링 집계 로직 (SampleController + OrderController 연계)

**단위 테스트**
- 모니터링 집계: REJECTED 제외, 재고 상태 판정, 잔여율 계산

**E2E 체크리스트**
- [ ] `[4]` 모니터링 서브메뉴: `[1]` 주문량 확인, `[2]` 재고량 확인
- [ ] REJECTED 주문이 집계에서 제외되는지 확인
- [ ] 재고 상태 배지 (여유·부족·고갈) + 프로그레스 바 색상 연동 확인
- [ ] 전체 시나리오 흐름: 시료 등록 → 주문 → 승인(생산) → 생산완료(재실행) → 출고 → 모니터링
- [ ] Debug 빌드 Post-Build Event로 전체 단위 테스트 자동 실행 확인

---

### 13.4 구현 우선순위 요약

| Phase | 항목 | 핵심 완료 조건 |
|-------|------|--------------|
| 1 | 도메인 모델 + Repository + Calculator | JSON 영속성 검증, State Machine 동작 |
| 2 | 시료 관리 + 콘솔 UI | 시료 CRUD 전체 화면 E2E |
| 3 | 주문 접수·승인·거절 | FIFO 재고 할당 포함 전체 흐름 E2E |
| 4 | 생산라인 + 출고 | 재실행 후 생산 완료 자동 반영 E2E |
| 5 | 모니터링 + 통합 | 전체 업무 흐름 시나리오 E2E |

---

## 14. 미결 사항 (Open Questions)

| 번호 | 질문 | 상태 |
|------|------|------|
| Q1 | JSON 파싱 라이브러리 선택 (nlohmann/json vs 자체 구현) | **결정: nlohmann/json** (DataPersistence PoC 기준) |
| Q2 | 생산라인 조회 시 "생산 완료" 처리 시점 — 메뉴 진입 시 자동 처리 여부 | **결정: [5] 메뉴 진입 시 경과 시간 계산 → 완료 조건 충족 항목 자동 CONFIRMED 전환** |
| Q3 | 주문번호 순번 카운터를 별도 파일로 관리할지 orders.json에서 파생할지 | **결정: orders.json에서 파생** (당일 주문 건수 카운트 + 1) |
| Q4 | 재고 차감 시점 | **결정: CONFIRMED 전환 시 차감** (PRODUCING 등록 시 차감 없음) |
