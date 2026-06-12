#include "SampleController.h"
#include "../view/SampleView.h"
#include "../view/MonitorView.h"
#include <algorithm>
#include <ostream>

SampleController::SampleController(IRepository<Sample>& sampleRepo,
                                   std::istream& in,
                                   std::ostream& out)
    : sampleRepo_(sampleRepo), in_(in), out_(out) {}

void SampleController::manageSamples() {
    while (true) {
        SampleView::printSubMenu(out_);
        int choice = SampleView::getSubMenuChoice(in_, out_);
        switch (choice) {
        case 1: registerSample(); break;
        case 2: listSamples();    break;
        case 3: searchByName();   break;
        case 0: return;
        default: out_ << "잘못된 선택입니다.\n"; break;
        }
    }
}

void SampleController::listSamples() {
    SampleView::printList(sampleRepo_.findAll(), out_);
}

void SampleController::registerSample() {
    Sample s = SampleView::inputNewSample(in_, out_);
    sampleRepo_.save(s);
    SampleView::printRegistered(s, out_);
}

void SampleController::searchByName() {
    std::string keyword = SampleView::inputSearchName(in_, out_);
    auto all = sampleRepo_.findAll();
    std::vector<Sample> found;
    for (const auto& s : all)
        if (s.name.find(keyword) != std::string::npos)
            found.push_back(s);

    if (found.empty())
        SampleView::printNotFound(out_);
    else
        SampleView::printList(found, out_);
}

void SampleController::showMonitoring(IOrderRepository& orderRepo) {
    auto orders  = orderRepo.findAll();
    auto samples = sampleRepo_.findAll();

    int reserved = 0, producing = 0, confirmed = 0, release = 0;
    for (const auto& o : orders) {
        switch (o.getStatus()) {
        case OrderStatus::RESERVED:  ++reserved;  break;
        case OrderStatus::PRODUCING: ++producing; break;
        case OrderStatus::CONFIRMED: ++confirmed; break;
        case OrderStatus::RELEASE:   ++release;   break;
        default: break;
        }
    }
    MonitorView::printOrderSummary(reserved, producing, confirmed, release, out_);

    std::vector<StockStatus> statuses;
    for (const auto& s : samples) {
        int pending = 0;
        for (const auto& o : orders) {
            if (o.sampleId == s.id &&
                (o.getStatus() == OrderStatus::RESERVED ||
                 o.getStatus() == OrderStatus::PRODUCING ||
                 o.getStatus() == OrderStatus::CONFIRMED))
                pending += o.quantity;
        }
        std::string statusStr;
        if (s.stock == 0)
            statusStr = "고갈";
        else if (pending > s.stock)
            statusStr = "부족";
        else
            statusStr = "여유";
        statuses.push_back({ s.id, s.name, statusStr, s.stock, pending });
    }
    MonitorView::printStockStatus(statuses, out_);
}
