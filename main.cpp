#include <windows.h>
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include "model/ProductionLine.h"
#include "model/SystemStatus.h"
#include "controller/SampleController.h"
#include "controller/OrderController.h"
#include "controller/ProductionController.h"
#include "controller/ReleaseController.h"
#include "view/MenuView.h"
#include "util/ProductionCalculator.h"
#include <iostream>
#include <algorithm>
#include <numeric>

static void enableAnsiColor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

static ProductionLine buildProductionLine(IOrderRepository& orderRepo,
                                          SampleRepository& sampleRepo) {
    ProductionLine line;
    auto producing = orderRepo.findByStatus(OrderStatus::PRODUCING);
    std::sort(producing.begin(), producing.end(),
        [](const Order& a, const Order& b) {
            return a.productionStartTime < b.productionStartTime;
        });
    for (const auto& o : producing) {
        auto s = sampleRepo.findById(o.sampleId);
        if (!s) continue;
        long long sec = ProductionCalculator::calcTotalTimeSeconds(s->avgProductionTime, o.productionQty);
        line.enqueue({ o.orderNo, o.sampleId, o.productionQty, sec, o.productionStartTime });
    }
    return line;
}

static SystemStatus buildSystemStatus(SampleRepository& sampleRepo, OrderRepository& orderRepo) {
    SystemStatus st;
    auto samples = sampleRepo.findAll();
    st.sampleCount = static_cast<int>(samples.size());
    for (const auto& s : samples)
        st.totalStock += s.stock;

    auto orders = orderRepo.findAll();
    for (const auto& o : orders) {
        if (o.getStatus() == OrderStatus::REJECTED) continue;
        ++st.orderCount;
        if (o.getStatus() == OrderStatus::PRODUCING)
            ++st.producingCount;
    }
    return st;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    enableAnsiColor();

    SampleRepository sampleRepo;
    OrderRepository  orderRepo;
    ProductionLine   line = buildProductionLine(orderRepo, sampleRepo);

    SampleController    sampleCtrl(sampleRepo, std::cin, std::cout);
    OrderController     orderCtrl(sampleRepo, orderRepo, line, std::cin, std::cout);
    ProductionController prodCtrl(sampleRepo, orderRepo, line, std::cout);
    ReleaseController   releaseCtrl(orderRepo, std::cin, std::cout);

    while (true) {
        SystemStatus status = buildSystemStatus(sampleRepo, orderRepo);
        MenuView::printMainMenu(status, std::cout);
        int choice = MenuView::getMenuChoice(std::cin, std::cout);
        switch (choice) {
        case 1: sampleCtrl.manageSamples();            break;
        case 2: orderCtrl.placeOrder();                break;
        case 3: orderCtrl.processApproval();           break;
        case 4: sampleCtrl.showMonitoring(orderRepo);  break;
        case 5: prodCtrl.showProductionLine();         break;
        case 6: releaseCtrl.processRelease();          break;
        case 0:
            std::cout << "  시스템을 종료합니다.\n";
            return 0;
        default:
            MenuView::printInvalidChoice(std::cout);
            break;
        }
    }
}
