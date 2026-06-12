#pragma once
#include <vector>
#include <optional>
#include <string>

template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;

    virtual std::vector<T>    findAll()                          const = 0;
    virtual std::optional<T>  findById(const std::string& id)   const = 0;
    virtual void              save(const T& entity)                    = 0;
    virtual void              update(const T& entity)                  = 0;
    virtual void              remove(const std::string& id)            = 0;
};
