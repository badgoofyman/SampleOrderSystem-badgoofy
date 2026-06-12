#pragma once
#include <string>

struct Sample {
    std::string id;
    std::string name;
    int         avgProductionTime = 0;  // min/ea, 1 이상
    double      yield             = 0.0; // 0 < yield <= 1.0
    int         stock             = 0;   // 0 이상
};
