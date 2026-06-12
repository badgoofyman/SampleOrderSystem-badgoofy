#include "ProductionController.h"
#include "../view/ProductionView.h"

ProductionController::ProductionController(IRepository<Sample>& sampleRepo,
                                           IOrderRepository&    orderRepo,
                                           ProductionLine&      line,
                                           std::ostream&        out)
    : sampleRepo_(sampleRepo), orderRepo_(orderRepo), line_(line), out_(out) {}

void ProductionController::processCompletedJobs() {
    while (!line_.isEmpty() && line_.isCurrentComplete()) {
        const auto& job = line_.current();

        auto orderOpt = orderRepo_.findById(job.orderNo);
        if (orderOpt) {
            Order o = *orderOpt;
            o.completeProduction();  // PRODUCING → CONFIRMED
            orderRepo_.update(o);

            auto sampleOpt = sampleRepo_.findById(job.sampleId);
            if (sampleOpt) {
                Sample s = *sampleOpt;
                s.stock += o.productionQty - o.quantity;
                sampleRepo_.update(s);
            }
            ProductionView::printCompleted(job.orderNo, out_);
        }
        line_.completeAndAdvance();
    }
}

void ProductionController::showProductionLine() {
    processCompletedJobs();
    ProductionView::printProductionLine(line_, out_);
}
