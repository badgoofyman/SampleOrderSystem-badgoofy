# SampleOrderSystem — 반도체 시료 생산주문관리 시스템

## 프로젝트 개요

가상의 반도체 회사 **S-Semi**의 시료(Sample) 생산주문관리 시스템.  
콘솔 기반으로 동작하며, 시료 등록부터 출고까지 전 과정을 관리한다.

- **Repository**: `SampleOrderSystem-badgoofy`
- **아키텍처**: MVC 패턴 (Model / View / Controller 패키지 분리)
- **DB**: JSON 파일 포맷 (데이터 영속성 보장)
- **UI**: 콘솔(터미널) 기반 메뉴 인터페이스

---

## 기술 스택

| 항목 | 내용 |
|------|------|
| 언어 | C++20 |
| IDE | Visual Studio 2022 (툴셋 v145) |
| 솔루션 | `SampleOrderSystem.slnx` |
| 플랫폼 | x64 / Win32, Debug / Release |
| 테스트 | gmock 1.11.0 (NuGet) |
| DB | JSON 파일 (별도 DB 서버 없음) |

---

## 폴더 구조

```
SampleOrderSystem/
├── model/
│   ├── Sample.h / Sample.cpp        # 시료 도메인 모델
│   ├── Order.h / Order.cpp          # 주문 도메인 모델
│   └── ProductionLine.h / .cpp      # 생산라인 도메인 모델
├── repository/
│   ├── IRepository.h                # 저장소 인터페이스 (Mock용)
│   ├── SampleRepository.h / .cpp    # 시료 JSON CRUD
│   └── OrderRepository.h / .cpp     # 주문 JSON CRUD
├── controller/
│   ├── SampleController.h / .cpp    # 시료 관리 흐름 제어
│   ├── OrderController.h / .cpp     # 주문 승인/거절 흐름 제어
│   ├── ProductionController.h / .cpp # 생산라인 흐름 제어
│   └── ReleaseController.h / .cpp   # 출고 처리 흐름 제어
├── view/
│   ├── MenuView.h / .cpp            # 메인 메뉴 출력
│   ├── SampleView.h / .cpp          # 시료 관련 화면
│   ├── OrderView.h / .cpp           # 주문 관련 화면
│   ├── MonitorView.h / .cpp         # 모니터링 화면
│   └── ProductionView.h / .cpp      # 생산라인 화면
├── util/
│   └── ProductionCalculator.h / .cpp # 생산량 계산 유틸
├── test/
│   └── *.cpp                        # gmock 기반 단위 테스트
├── data/
│   ├── samples.json                 # 시료 DB
│   └── orders.json                  # 주문 DB
├── packages/
│   └── gmock.1.11.0/                # NuGet 패키지
└── main.cpp                         # 진입점
```

---

## 역할별 흐름

```
고객 → (시료 요청) → 주문담당자 → (주문서 전달) → 생산담당자
                         ↑                              |
                         └─────── (승인 / 거절) ────────┘
```

---

## 도메인 모델

### 시료 (Sample)

| 필드 | 타입 | 설명 |
|------|------|------|
| id | string | 고유 식별자 (예: S-001) |
| name | string | 시료명 (예: 실리콘 웨이퍼-8인치) |
| avgProductionTime | int | 단위당 생산 소요 시간 (min/ea) |
| yield | double | 수율 (예: 0.92) |
| stock | int | 현재 보유 수량 |

### 주문 (Order)

| 필드 | 타입 | 설명 |
|------|------|------|
| orderNo | string | 주문번호 (예: ORD-20260416-0043) |
| sampleId | string | 주문 대상 시료 ID |
| customerName | string | 주문 고객명 |
| quantity | int | 주문 수량 |
| status | OrderStatus | 주문 상태 (열거형) |

---

## 주문 상태 흐름 (State Machine)

```
RESERVED → (승인) → 재고 충분 → CONFIRMED → RELEASE
                 → 재고 부족 → PRODUCING → CONFIRMED → RELEASE
         → (거절) → REJECTED
```

| 상태 | 의미 |
|------|------|
| `RESERVED` | 주문 접수 |
| `REJECTED` | 주문 거절 (모니터링 집계 제외) |
| `PRODUCING` | 승인 완료 + 재고 부족으로 생산 중 |
| `CONFIRMED` | 승인 완료 + 출고 대기 중 |
| `RELEASE` | 출고 완료 |

- 상태 전환은 State Machine으로 관리하여 불법 전환 방지
- 허용된 전환 외의 상태 변경은 예외 처리

---

## 생산라인 계산 공식

```
실 생산량 = ceil(부족분 / (수율 × 0.9))
총 생산시간 = 평균 생산시간 × 실 생산량
```

- 계산 로직은 `util/ProductionCalculator`에 격리
- 스케줄링 전략: **FIFO** (선입선출)
- 생산 라인은 단일 라인, 시료를 하나씩 처리
- 생산 완료 시 `PRODUCING` → `CONFIRMED` 자동 전환

---

## 메인 메뉴 구조

| 번호 | 메뉴 | 담당 Controller |
|------|------|----------------|
| [1] | 시료 관리 | `SampleController` |
| [2] | 시료 주문 | `OrderController` |
| [3] | 주문 승인/거절 | `OrderController` |
| [4] | 모니터링 | `SampleController` + `OrderController` |
| [5] | 생산라인 조회 | `ProductionController` |
| [6] | 출고 처리 | `ReleaseController` |
| [0] | 종료 | — |

### 모니터링 재고 상태 표기

| 표기 | 조건 |
|------|------|
| **여유** | 주문 대비 재고 충분 |
| **부족** | 주문 대비 재고 수량 부족 |
| **고갈** | 재고 수량이 0 |

---

## 의존 방향 (단방향 엄수)

```
View → Controller → Repository → Model
```

- View는 Controller만 호출, Repository·Model 직접 접근 금지
- Controller는 Repository 인터페이스를 통해 데이터 접근
- Model은 순수 데이터 구조 + 비즈니스 로직만 보유

---

## 테스트 가이드

- 테스트 파일은 `test/` 디렉토리에 작성
- gmock 헤더: `#include <gmock/gmock.h>`
- 각 Controller는 `IRepository` Mock을 주입하여 격리 테스트
- `ProductionCalculator` 계산 공식은 수치 기반 단위 테스트 필수
- 상태 머신의 합법·불법 전환 케이스 모두 테스트

---

## 코드 작성 원칙

- MVC + Repository 패키지 경계 엄격히 유지
- JSON 파일 입출력은 DataPersistence PoC 구조를 그대로 적용
- 생산량 계산 공식은 `ProductionCalculator` 유틸로 분리
- 주문 상태 전환은 State Machine 방식으로 관리하여 불법 전환 방지
- REJECTED 주문은 모니터링 집계에서 제외
- CleanCode 원칙 준수: 함수는 단일 책임, 명확한 네이밍
