#include "SampleController.h"
#include "../view/SampleView.h"
#include "../view/MonitorView.h"
#include <algorithm>
#include <cctype>
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
        default: out_ << "  잘못된 선택입니다.\n"; break;
        }
    }
}

void SampleController::listSamples() {
    SampleView::printListPaged(sampleRepo_.findAll(), in_, out_);
}

void SampleController::registerSample() {
    out_ << "\n  ── 시료 등록 ──\n";

    // 중복 ID 검사 루프
    std::string id;
    while (true) {
        id = SampleView::inputSampleId(in_, out_);
        if (id.empty()) return;
        if (!sampleRepo_.findById(id)) break;
        SampleView::printDuplicateId(out_);
    }

    std::string name = SampleView::inputSampleName(in_, out_);
    int time = SampleView::inputProductionTime(in_, out_);

    // 수율 범위 검사 루프 (0.0 < yield <= 1.0)
    double yield;
    while (true) {
        yield = SampleView::inputYield(in_, out_);
        if (yield > 0.0 && yield <= 1.0) break;
        SampleView::printInvalidYield(out_);
    }

    int stock = SampleView::inputStock(in_, out_);

    Sample s;
    s.id = id; s.name = name; s.avgProductionTime = time; s.yield = yield; s.stock = stock;
    sampleRepo_.save(s);
    SampleView::printRegistered(s, out_);
}

void SampleController::searchByName() {
    std::string keyword = SampleView::inputSearchName(in_, out_);

    // 대소문자 무시 검색
    std::string kwLower = keyword;
    std::transform(kwLower.begin(), kwLower.end(), kwLower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    auto all = sampleRepo_.findAll();
    std::vector<Sample> found;
    for (const auto& s : all) {
        std::string nameLower = s.name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (nameLower.find(kwLower) != std::string::npos)
            found.push_back(s);
    }

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
