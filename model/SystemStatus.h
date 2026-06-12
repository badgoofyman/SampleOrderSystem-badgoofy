#pragma once

struct SystemStatus {
    int sampleCount    = 0;  // 등록 시료 수
    int totalStock     = 0;  // 총 재고 합산
    int orderCount     = 0;  // 전체 주문 수 (REJECTED 제외)
    int producingCount = 0;  // PRODUCING 상태 주문 수
};
