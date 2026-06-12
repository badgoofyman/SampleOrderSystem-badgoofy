#pragma once
#include "../repository/IRepository.h"
#include "../repository/IOrderRepository.h"
#include "../model/Sample.h"
#include <iosfwd>

class SampleController {
public:
    SampleController(IRepository<Sample>& sampleRepo,
                     std::istream& in,
                     std::ostream& out);

    void manageSamples();
    void showMonitoring(IOrderRepository& orderRepo);

private:
    IRepository<Sample>& sampleRepo_;
    std::istream&        in_;
    std::ostream&        out_;

    void listSamples();
    void registerSample();
    void searchByName();
};
