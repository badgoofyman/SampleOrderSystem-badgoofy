#pragma once
#include <iosfwd>

class MenuView {
public:
    static void printMainMenu(std::ostream& out);
    static int  getMenuChoice(std::istream& in, std::ostream& out);
    static void printInvalidChoice(std::ostream& out);
};
