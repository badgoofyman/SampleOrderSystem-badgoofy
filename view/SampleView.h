#pragma once
#include "../model/Sample.h"
#include <iosfwd>
#include <vector>
#include <string>

class SampleView {
public:
    static void        printSubMenu(std::ostream& out);
    static int         getSubMenuChoice(std::istream& in, std::ostream& out);
    static void        printList(const std::vector<Sample>& samples, std::ostream& out);
    static Sample      inputNewSample(std::istream& in, std::ostream& out);
    static std::string inputSearchName(std::istream& in, std::ostream& out);
    static void        printNotFound(std::ostream& out);
    static void        printRegistered(const Sample& s, std::ostream& out);
};
