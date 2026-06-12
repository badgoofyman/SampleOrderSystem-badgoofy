#pragma once
#include "../repository/IRepository.h"
#include "../repository/IOrderRepository.h"
#include "../model/Sample.h"
#include "../model/ProductionLine.h"
#include <iosfwd>
#include <string>

class OrderController {
public:
    OrderController(IRepository<Sample>& sampleRepo,
                    IOrderRepository&    orderRepo,
                    ProductionLine&      line,
                    std::istream&        in,
                    std::ostream&        out);

    void placeOrder();
    void processApproval();

private:
    IRepository<Sample>& sampleRepo_;
    IOrderRepository&    orderRepo_;
    ProductionLine&      line_;
    std::istream&        in_;
    std::ostream&        out_;

    std::string generateOrderNo() const;
    void        approveOrder(Order& o, const Sample& s, int available);
};
