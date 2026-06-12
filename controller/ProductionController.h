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
                         std::istream&        in,
                         std::ostream&        out);

    void showProductionLine();

private:
    IRepository<Sample>& sampleRepo_;
    IOrderRepository&    orderRepo_;
    ProductionLine&      line_;
    std::istream&        in_;
    std::ostream&        out_;

    void processCompletedJobs();
};
