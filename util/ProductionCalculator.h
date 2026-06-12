#pragma once

namespace ProductionCalculator {

// 실 생산량 = ceil(shortage / (yieldRate * 0.9))
int       calcProductionQty(int shortage, double yieldRate);

// 총 생산시간 (분)
int       calcTotalTimeMinutes(int avgTime, int productionQty);

// 총 생산시간 (초)
long long calcTotalTimeSeconds(int avgTime, int productionQty);

} // namespace ProductionCalculator
