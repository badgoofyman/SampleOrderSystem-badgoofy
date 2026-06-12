# Phase 4 검토 체크리스트

**브랜치**: `phase/4-monitor-production`  
**검토 시점**: PR 리뷰 전 로컬에서 직접 확인

---

## 1. 빌드 및 테스트

- [ ] Debug|x64 빌드 성공 — 오류·경고 0건
- [ ] Release|x64 빌드 성공
- [ ] 모든 테스트 PASSED — 기존 77개 포함 신규 테스트 전체 통과
- [ ] 실패 테스트 0건

---

## 2. MonitorView UX 완성

### 2-1. 주문 현황 박스
- [ ] 박스 테두리(`┌─┐│└─┘`) 스타일 출력
- [ ] RESERVED 배지 — Blue
- [ ] PRODUCING 배지 — Orange
- [ ] CONFIRMED 배지 — Green
- [ ] RELEASE 배지 — Purple
- [ ] 각 상태별 건수 Yellow 강조

### 2-2. 재고 현황 테이블
- [ ] 박스 테두리(`┌─┬─┐`) 스타일 출력
- [ ] **여유** 상태 — Green 출력
- [ ] **부족** 상태 — Yellow 출력
- [ ] **고갈** 상태 — Red 출력 + 재고 수치도 Red
- [ ] ID, 이름, 재고, 주문대기, 상태 5개 컬럼 유지

---

## 3. ProductionView UX 완성

### 3-1. 생산중 항목
- [ ] `[생산중]` 레이블 Orange 강조
- [ ] 주문번호 / 시료 ID / 생산량 출력
- [ ] 완료 예정 시각 (`YYYY-MM-DD HH:MM:SS`)
- [ ] **잔여 시간** 계산 및 출력 (`완료까지 N분` 형식)
  - `잔여 = finishTime - time(nullptr)` (초 → 분 변환)
  - 이미 완료된 경우 `완료됨` 표시

### 3-2. 대기 항목
- [ ] `[대기 N]` 레이블 Yellow 강조
- [ ] 예상 시작 시각 출력 (이전 잡 완료 시각 = 본 잡의 startTime)

### 3-3. 빈 라인
- [ ] "현재 생산 중인 주문이 없습니다." — Yellow 출력

### 3-4. 생산 완료 자동 처리 메시지
- [ ] `[orderNo] → CONFIRMED` — Green 출력

---

## 4. SampleView 재고 0 강조

- [ ] `printList` 에서 `stock == 0` 행 → Red 컬러
- [ ] `printListPaged` 에서 `stock == 0` 행 → Red 컬러
- [ ] 재고 수치 `0` 숫자도 Red

---

## 5. 테스트 검토

### 5-1. MonitorViewTest (신규)
- [ ] `PrintOrderSummary_ContainsStatusLabels` — 상태 레이블 포함 확인
- [ ] `PrintStockStatus_ExcessStock_ShowsGreen` — 여유 상태 Green ANSI 포함
- [ ] `PrintStockStatus_InsufficientStock_ShowsYellow` — 부족 상태 Yellow ANSI 포함
- [ ] `PrintStockStatus_DepletedStock_ShowsRed` — 고갈 상태 Red ANSI 포함

### 5-2. ProductionViewTest (신규)
- [ ] `PrintEmpty_ShowsMessage` — 빈 라인 메시지 확인
- [ ] `PrintCompleted_ShowsOrderNo` — 완료 메시지 주문번호 포함
- [ ] `PrintProductionLine_ActiveJob_ShowsRemainingTime` — 잔여시간 텍스트 포함
- [ ] `PrintProductionLine_MultipleJobs_ShowsFifoOrder` — 순서 확인

### 5-3. SampleViewTest (신규)
- [ ] `PrintList_ZeroStock_ContainsRedAnsi` — Red ANSI 코드 포함 확인
- [ ] `PrintList_NonZeroStock_NoRedAnsi` — 일반 항목에 Red 없음 확인

### 5-4. ProductionControllerTest (추가)
- [ ] `ShowProductionLine_MultipleJobs_FifoOrderPreserved` — FIFO 순서 확인
- [ ] `ShowProductionLine_CompletedJob_StockUpdatedCorrectly` — 재고 반영 정확도

---

## 6. UI 컬러 일관성 최종 점검

| 대상 | 색상 | 확인 |
|------|------|------|
| `여유` 재고 상태 | Green | [ ] |
| `부족` 재고 상태 | Yellow | [ ] |
| `고갈` 재고 상태 | Red | [ ] |
| 재고 0 수치 | Red | [ ] |
| `[생산중]` 레이블 | Orange | [ ] |
| `[대기 N]` 레이블 | Yellow | [ ] |
| 생산완료 메시지 | Green | [ ] |
| RESERVED 배지 | Blue | [ ] |
| PRODUCING 배지 | Orange | [ ] |
| CONFIRMED 배지 | Green | [ ] |
| RELEASE 배지 | Purple | [ ] |

---

## 7. 수동 시나리오 확인

### 시나리오 A — 모니터링 화면
1. 시료 2종 등록, 주문 3건 (RESERVED·PRODUCING·CONFIRMED 각 1건)
2. [4] 모니터링 → 주문 현황 배지 컬러 확인
3. 재고 0인 시료 포함 → 재고 상태 `고갈` Red 출력 확인

### 시나리오 B — 생산라인 화면
1. PRODUCING 주문 2건 (startTime 순서 다르게 설정)
2. [5] 생산라인 조회 → `[생산중]` 1건, `[대기 1]` 1건 출력 확인
3. 잔여 시간 표시 확인 (미래 완료 예정 → `완료까지 N분`)
4. 완료된 잡 → `→ CONFIRMED` 자동 처리 + Green 메시지 확인

### 시나리오 C — 시료 목록 재고 0 강조
1. 재고 0인 시료와 일반 시료 혼합 등록
2. [1] → [2] 목록 조회 → 재고 0 행이 Red로 출력 확인

---

## 8. Git 이력 검토

- [ ] TDD 순서 준수 — `test:` 커밋 후 `feat:` 커밋
- [ ] `chore:` 커밋으로 vcxproj 등록 분리
- [ ] 불필요한 임시 커밋 없음

---

## 9. PR 발행 전 최종 확인

- [ ] 위 1~8번 항목 전체 통과
- [ ] `phase/4-monitor-production` 브랜치가 최신 `main`과 충돌 없음
- [ ] PR 본문에 테스트 결과 포함
