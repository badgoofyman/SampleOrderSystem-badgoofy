#pragma once
#include "../repository/IOrderRepository.h"
#include <iosfwd>

class ReleaseController {
public:
    ReleaseController(IOrderRepository& orderRepo,
                      std::istream&     in,
                      std::ostream&     out);

    void processRelease();

private:
    IOrderRepository& orderRepo_;
    std::istream&     in_;
    std::ostream&     out_;
};
