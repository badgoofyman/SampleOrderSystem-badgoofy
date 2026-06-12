#pragma once
#include "IRepository.h"
#include "../model/Sample.h"
#include <string>

class SampleRepository : public IRepository<Sample> {
public:
    explicit SampleRepository(const std::string& filePath = "data/samples.json");

    std::vector<Sample>   findAll()                          const override;
    std::optional<Sample> findById(const std::string& id)   const override;
    void                  save(const Sample& sample)               override;
    void                  update(const Sample& sample)             override;
    void                  remove(const std::string& id)            override;

private:
    std::string filePath_;
};
