#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include "model/ProductionLine.h"
#include "controller/SampleController.h"
#include "controller/OrderController.h"
#include "controller/ProductionController.h"
#include "controller/ReleaseController.h"
#include "view/MenuView.h"
#include "util/ProductionCalculator.h"
#include <iostream>
#include <algorithm>

static ProductionLine buildProductionLine(OrderRepository& orderRepo, SampleRepository& sampleRepo) {
    ProductionLine line;
    auto producing = orderRepo.findByStatus(OrderStatus::PRODUCING);
    std::sort(producing.begin(), producing.end(),
        [](const Order& a, const Order& b) { return a.productionStartTime < b.productionStartTime; });

    for (const auto& o : producing) {
        auto s = sampleRepo.findById(o.sampleId);
        if (!s) continue;
        long long sec = ProductionCalculator::calcTotalTimeSeconds(s->avgProductionTime, o.productionQty);
        line.enqueue({ o.orderNo, o.sampleId, o.productionQty, sec, o.productionStartTime });
    }
    return line;
}

int main() {
    SampleRepository sampleRepo;
    OrderRepository  orderRepo;
    ProductionLine   line = buildProductionLine(orderRepo, sampleRepo);

    SampleController    sampleCtrl(sampleRepo, std::cin, std::cout);
    OrderController     orderCtrl(sampleRepo, orderRepo, line, std::cin, std::cout);
    ProductionController prodCtrl(sampleRepo, orderRepo, line, std::cin, std::cout);
    ReleaseController   releaseCtrl(orderRepo, std::cin, std::cout);

    while (true) {
        MenuView::printMainMenu(std::cout);
        int choice = MenuView::getMenuChoice(std::cin, std::cout);
        switch (choice) {
        case 1: sampleCtrl.manageSamples();               break;
        case 2: orderCtrl.placeOrder();                   break;
        case 3: orderCtrl.processApproval();              break;
        case 4: sampleCtrl.showMonitoring(orderRepo);     break;
        case 5: prodCtrl.showProductionLine();            break;
        case 6: releaseCtrl.processRelease();             break;
        case 0:
            std::cout << "시스템을 종료합니다.\n";
            return 0;
        default:
            MenuView::printInvalidChoice(std::cout);
            break;
        }
    }
}
