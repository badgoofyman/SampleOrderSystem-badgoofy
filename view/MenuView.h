#pragma once
#include "../model/SystemStatus.h"
#include <iosfwd>

class MenuView {
public:
    static void printMainMenu(const SystemStatus& status, std::ostream& out);
    static int  getMenuChoice(std::istream& in, std::ostream& out);
    static void printInvalidChoice(std::ostream& out);
};
