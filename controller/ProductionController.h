#pragma once
#include "../repository/IRepository.h"
#include "../repository/IOrderRepository.h"
#include "../model/Sample.h"
#include "../model/ProductionLine.h"
#include <iosfwd>

class ProductionController {
public:
    ProductionController(IRepository<Sample>& sampleRepo,
                         IOrderRepository&    orderRepo,
                         ProductionLine&      line,
                         std::ostream&        out);

    void showProductionLine();
    void processCompletedJobs();

private:
    IRepository<Sample>& sampleRepo_;
    IOrderRepository&    orderRepo_;
    ProductionLine&      line_;
    std::ostream&        out_;
};
