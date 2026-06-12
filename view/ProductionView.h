#pragma once
#include "../model/ProductionLine.h"
#include <iosfwd>

class ProductionView {
public:
    static void printProductionLine(const ProductionLine& line, std::ostream& out);
    static void printEmpty(std::ostream& out);
    static void printCompleted(const std::string& orderNo, std::ostream& out);
};
