#include "ProductionCalculator.h"
#include <cmath>
#include <stdexcept>

namespace ProductionCalculator {

static constexpr double SAFETY_FACTOR = 0.9;

int calcProductionQty(int shortage, double yieldRate) {
    if (shortage <= 0)
        throw std::invalid_argument("shortage must be positive");
    if (yieldRate <= 0.0 || yieldRate > 1.0)
        throw std::invalid_argument("yieldRate must be in (0.0, 1.0]");
    return static_cast<int>(std::ceil(shortage / (yieldRate * SAFETY_FACTOR)));
}

int calcTotalTimeMinutes(int avgTime, int productionQty) {
    if (avgTime <= 0)
        throw std::invalid_argument("avgTime must be positive");
    return avgTime * productionQty;
}

long long calcTotalTimeSeconds(int avgTime, int productionQty) {
    return static_cast<long long>(calcTotalTimeMinutes(avgTime, productionQty)) * 60LL;
}

} // namespace ProductionCalculator
