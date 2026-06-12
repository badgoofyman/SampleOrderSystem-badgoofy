# Phase 3 검토 체크리스트

**브랜치**: `phase/3-order`  
**검토 시점**: PR 리뷰 전 로컬에서 직접 확인

---

## 1. 빌드 및 테스트 자동 확인

- [ ] **Debug|x64 빌드 성공** — 오류·경고 0건
- [ ] **Release|x64 빌드 성공**
- [ ] **모든 테스트 PASSED** — 이전 71개 포함, 신규 테스트 모두 통과
- [ ] **실패 테스트 0건**

---

## 2. ConsoleColor 유틸 검토

- [ ] `util/ConsoleColor.h` 파일 존재
- [ ] 헤더 전용 구현 (`#pragma once`, `namespace ConsoleColor`)
- [ ] 정의된 컬러 상수: `RESET`, `WHITE`, `CYAN`, `BLUE`, `YELLOW`, `GREEN`, `RED`, `PURPLE`, `ORANGE`
- [ ] `print(out, text, color)`, `println(out, text, color)` 유틸 함수 존재
- [ ] Controller·Model·Repository 파일에서 `ConsoleColor` 직접 사용 없음 — View에서만 호출

---

## 3. main.cpp 개선 검토

- [ ] `enableAnsiColor()` 호출 — `ENABLE_VIRTUAL_TERMINAL_PROCESSING` 플래그 설정
- [ ] `SetConsoleOutputCP(CP_UTF8)` + `SetConsoleCP(CP_UTF8)` 유지
- [ ] `buildSystemStatus()` 함수 존재 — Sample/Order 데이터 집계
- [ ] 시스템 현황 항목: `sampleCount`, `totalStock`, `orderCount`(REJECTED 제외), `producingCount`
- [ ] `MenuView::printMainMenu`에 `SystemStatus` 또는 집계 수치 전달

---

## 4. 메인 화면 (MenuView) 검토

- [ ] ASCII Art "S-Semi" 로고 출력 — 6줄 블록 폰트 하드코딩
- [ ] 현재 날짜·시간 출력 (YYYY-MM-DD HH:MM:SS 형식)
- [ ] 시스템 현황 테이블:
  - 등록 시료 수 (노란색)
  - 총 재고 (노란색)
  - 전체 주문 수 (노란색, REJECTED 제외)
  - 생산라인 대기 건수 (노란색)
- [ ] 메뉴 2열 배치:
  ```
    [1] 시료 관리                          [2] 시료 주문
    [3] 주문 승인/거절                     [4] 모니터링
    [5] 생산라인 조회                      [6] 출고 처리
    [0] 종료
  ```
- [ ] 메뉴 번호 `[N]` — Cyan 색상
- [ ] 구분선 `=====` / `-----` 출력

---

## 5. 시료 관리 (SampleView + SampleController) 검토

### 5-1. 시료 등록 유효성 검사
- [ ] 중복 ID 입력 시 오류 메시지(빨간색) 출력 후 **재입력 루프** 동작
  - 오류 메시지: `오류: 이미 존재하는 시료 ID입니다. 다시 입력하세요.`
- [ ] 수율 범위 위반(`yield ≤ 0.0` 또는 `yield > 1.0`) 시 오류 메시지(빨간색) 후 **재입력 루프**
- [ ] 빈 입력 처리 (고객명·ID 등 필수 필드)
- [ ] 등록 완료 메시지 — 초록색

### 5-2. 시료 목록 페이지네이션
- [ ] 한 페이지 5행 표시
- [ ] 5행 초과 시 `[N] 다음 페이지` 안내 출력
- [ ] `N` 입력 시 다음 페이지로 이동
- [ ] 마지막 페이지에서 `[N]` 안내 없음
- [ ] 재고 0인 항목 — 빨간색으로 강조

### 5-3. 시료 이름 검색
- [ ] 대소문자 무시 부분 일치 검색 (`tolower` 또는 `transform` 사용)
- [ ] 검색 결과 없을 시: `검색 결과가 없습니다.` (노란색)

### 5-4. 테스트 추가 항목
- [ ] `RegisterSample_DuplicateId_RepromptsAndSucceeds` — 중복 후 정상 등록
- [ ] `RegisterSample_InvalidYield_RepromptsAndSucceeds` — 수율 오류 후 정상 등록
- [ ] `SearchByName_CaseInsensitive_FindsMatch` — 대소문자 무시 검색

---

## 6. 주문 접수 ([2], OrderController + OrderView) 검토

### 6-1. 입력 흐름
- [ ] 입력 순서: 시료 ID → 고객명 → 주문 수량
- [ ] 존재하지 않는 시료 ID → 오류 메시지 후 **재입력 루프** (기존: 1회만 처리)
- [ ] 빈 고객명 → 오류 메시지 후 **재입력 루프**
- [ ] 수량 0 이하 → 오류 메시지 후 **재입력 루프**

### 6-2. 확인 화면 (Y/N)
- [ ] 입력 완료 후 확인 화면 출력:
  ```
  입력 내용 확인
  시료        <시료명>   (<시료 ID>)
  고객        <고객명>
  수량        <수량> ea
  [Y] 예약 접수    [N] 취소
  ```
- [ ] Y 선택 → 주문번호 생성 + RESERVED 저장
- [ ] N 선택 → 취소 메시지 출력, 저장 없음

### 6-3. 주문번호 당일 기준 순번
- [ ] 주문번호 형식: `ORD-YYYYMMDD-NNNN`
- [ ] `YYYYMMDD` — 접수 당일 날짜
- [ ] `NNNN` — `orders.json`에서 동일 날짜 접두사(`ORD-YYYYMMDD-`) 카운트 + 1

### 6-4. 완료 화면
- [ ] 완료 메시지 초록색
- [ ] 주문번호 출력 (자홍색 강조)
- [ ] 상태 배지 `[ RESERVED ]` — 파란색
- [ ] ※ 승인 메뉴 안내 문구 — 노란색

### 6-5. 테스트 추가 항목
- [ ] `PlaceOrder_YesConfirm_SavesReserved` — Y 선택 시 save 호출
- [ ] `PlaceOrder_NoConfirm_NoSave` — N 선택 시 save 미호출
- [ ] `PlaceOrder_InvalidSampleId_Reprompts` — 재입력 루프 동작

---

## 7. 주문 승인/거절 ([3], OrderController + OrderView) 검토

### 7-1. 번호 선택 방식
- [ ] RESERVED 목록을 번호(1, 2, 3...)로 표시
  ```
  번호    주문번호       고객              시료           수량      상태
  [1]     ORD-0041  ...
  [2]     ORD-0042  ...
  ```
- [ ] `승인할 번호 > ` 프롬프트 출력
- [ ] 범위 외 번호 입력 → 오류 메시지 후 재입력

### 7-2. 재고 확인 화면
- [ ] 선택 후 재고 확인 화면 출력:
  ```
  시료          <시료명>    현재 재고   <stock> ea
  주문 수량     <qty> ea    부족분      <shortage> ea
  ```
- [ ] 재고 충분 시: `재고 충분. 즉시 출고 대기로 전환합니다.`
- [ ] 재고 부족 시: `재고 부족. 부족분 <N> ea (실생산량 <prodQty> ea / <totalMin> min)`
  - 실생산량: `ceil(부족분 / (수율 × 0.9))`
  - 총생산시간: `평균생산시간 × 실생산량` (분 단위)

### 7-3. FIFO 가용 재고 계산
- [ ] 가용 재고 = `현재 재고 - CONFIRMED 상태 동일 시료 미출고 수량 합산`
- [ ] PRODUCING 중인 주문은 가용 재고 계산에서 **제외**

### 7-4. 결과 화면
- [ ] 승인 결과 초록색 `승인 완료.`
- [ ] 상태 변경 출력: `RESERVED → [ CONFIRMED ]` 또는 `RESERVED → [ PRODUCING ]`
- [ ] 거절 결과: `주문 거절 처리 완료.` + `RESERVED → REJECTED`

### 7-5. 테스트 추가 항목
- [ ] `ProcessApproval_SelectByNumber_ApprovesCorrectOrder` — 번호로 정확한 주문 선택
- [ ] `ProcessApproval_FifoStock_ReducedByConfirmed` — FIFO 가용 재고 (CONFIRMED 차감 반영)
- [ ] `ProcessApproval_StockSufficient_ShowsConfirmScreen` — 재고 충분 화면 출력 확인
- [ ] `ProcessApproval_StockInsufficient_ShowsProductionInfo` — 부족분·실생산량·총생산시간 출력

---

## 8. 출고 처리 ([6], ReleaseController + OrderView) 검토

### 8-1. 번호 선택 방식
- [ ] CONFIRMED 목록을 번호(1, 2, 3...)로 표시
  ```
  번호    주문번호       고객          시료           수량
  [1]     ORD-0042  ...
  ```
- [ ] `출고할 번호 > ` 프롬프트 출력
- [ ] 범위 외 번호 → 오류 메시지 후 재입력

### 8-2. 결과 화면
- [ ] 완료 메시지 초록색 `출고 처리 완료.`
- [ ] 출력 항목: 주문번호, 출고수량, 처리일시(`YYYY-MM-DD HH:MM:SS`), 상태 변경
- [ ] 상태 변경: `CONFIRMED → [ RELEASE ]` — RELEASE는 보라색

### 8-3. 테스트 추가 항목
- [ ] `ProcessRelease_SelectByNumber_ReleasesCorrectOrder` — 번호 선택
- [ ] `ProcessRelease_ShowsResultWithTimestamp` — 결과 화면(주문번호, 수량, 처리일시 포함)

---

## 9. UI 컬러 일관성 검토

| 대상 | 색상 | 확인 |
|------|------|------|
| 메뉴 번호 `[N]` | Cyan (`\033[96m`) | [ ] |
| 테이블 헤더 | Bright Blue (`\033[94m`) | [ ] |
| 강조 수치 | Yellow (`\033[93m`) | [ ] |
| 완료 메시지 | Green (`\033[92m`) | [ ] |
| 오류 메시지 | Red (`\033[91m`) | [ ] |
| 안내/경고 `※` | Yellow (`\033[93m`) | [ ] |
| `RESERVED` 배지 | Blue (`\033[94m`) | [ ] |
| `PRODUCING` 배지 | Orange (`\033[33m`) | [ ] |
| `CONFIRMED` 배지 | Green (`\033[92m`) | [ ] |
| `RELEASE` 배지 | Purple (`\033[95m`) | [ ] |
| 재고 0 항목 | Red (`\033[91m`) | [ ] |

---

## 10. 전체 흐름 수동 실행 확인

### 시나리오 A — 정상 주문 접수 및 취소
1. [2] 시료 주문 → 시료 ID 입력 → 고객명 → 수량 → 확인 화면(Y/N)
2. N 선택 → 취소 메시지 확인, orders.json 저장 없음 확인
3. 다시 [2] → 동일 입력 → Y 선택 → 주문번호 `ORD-<today>-0001` 생성 확인

### 시나리오 B — 중복 ID 및 수율 오류 처리
1. [1] 시료 등록 → S-001 입력 (이미 존재) → 오류 메시지 + 재입력 확인
2. 새 ID로 재입력 → 수율 `1.5` 입력 → 오류 메시지 + 재입력 확인
3. 수율 `0.9` 입력 → 정상 등록 확인

### 시나리오 C — FIFO 재고 할당 확인
1. 시료 S-001 재고 15 설정
2. 주문 A: S-001 10개 접수 → 승인 → CONFIRMED (재고 10 차감, 재고 5 남음)
3. 주문 B: S-001 10개 접수 → 승인 → 가용 재고 5, 부족분 5 → PRODUCING 확인
4. [4] 모니터링에서 S-001 재고 5, 상태 `부족` 확인

### 시나리오 D — 주문 승인 번호 선택
1. RESERVED 주문 3개 준비
2. [3] 승인 화면 → 번호 목록 확인 → `2` 입력 → 2번 주문 처리 확인
3. 잘못된 번호(예: `9`) 입력 → 오류 메시지 + 재입력 확인

### 시나리오 E — 출고 번호 선택
1. CONFIRMED 주문 2개 준비
2. [6] 출고 화면 → `1` 입력 → 1번 주문 RELEASE 처리 확인
3. 처리일시 형식(`YYYY-MM-DD HH:MM:SS`) 확인

### 시나리오 F — 당일 순번 확인
1. 첫 주문: `ORD-20260612-0001`
2. 두 번째 주문: `ORD-20260612-0002`
3. 다음 날 시뮬레이션 (날짜 변경 불가 → JSON 수동 수정 or 코드 확인으로 대체)

---

## 11. JSON 영속성 검토

- [ ] 주문 접수 시 Y 선택 후 `data/orders.json`에 즉시 저장 확인
- [ ] 주문 접수 N 취소 시 `data/orders.json` 변경 없음 확인
- [ ] 승인 후 `status`, `productionStartTime` (부족 시) 정상 저장
- [ ] 출고 후 `status: RELEASE`, `releasedAt` 타임스탬프 저장

---

## 12. Git 이력 검토

`git log --oneline` 확인:
- [ ] TDD 순서 준수 — `test:` 커밋이 `feat:` 커밋보다 앞에 위치
- [ ] Step 0~6 커밋 11개 순서 정상
- [ ] 불필요한 임시 커밋 없음

---

## 13. 코드 품질 검토

- [ ] `ConsoleColor` 호출이 View 레이어에만 존재
- [ ] 재입력 루프가 무한루프 방지 설계 (`while(true)` + `break`)
- [ ] `generateOrderNo()` — 날짜 변경 후에도 순번 1번 재시작
- [ ] FIFO 가용 재고 계산에서 PRODUCING 주문 제외 확인
- [ ] 함수 길이 30줄 이내 (단일 책임 유지)

---

## 14. PR 발행 전 최종 확인

- [ ] 위 1~13번 항목 전체 통과
- [ ] `phase/3-order` 브랜치가 최신 `main`과 충돌 없음
- [ ] PR 본문에 테스트 결과 (Debug|x64 테스트 총 수 PASSED) 포함
- [ ] PR 제목: `[Phase 3] UI 완성 + 주문 접수/승인/거절 PRD 준수`
