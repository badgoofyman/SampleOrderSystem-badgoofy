#pragma once
#include "../model/Sample.h"
#include <iosfwd>
#include <vector>
#include <string>

class SampleView {
public:
    static void        printSubMenu(std::ostream& out);
    static int         getSubMenuChoice(std::istream& in, std::ostream& out);

    // 목록 출력
    static void printList(const std::vector<Sample>& samples, std::ostream& out);
    static void printListPaged(const std::vector<Sample>& samples,
                               std::istream& in, std::ostream& out);

    // 등록 입력 (개별 필드)
    static std::string inputSampleId(std::istream& in, std::ostream& out);
    static std::string inputSampleName(std::istream& in, std::ostream& out);
    static int         inputProductionTime(std::istream& in, std::ostream& out);
    static double      inputYield(std::istream& in, std::ostream& out);
    static int         inputStock(std::istream& in, std::ostream& out);

    // 오류 메시지
    static void printDuplicateId(std::ostream& out);
    static void printInvalidYield(std::ostream& out);

    // 검색
    static std::string inputSearchName(std::istream& in, std::ostream& out);
    static void        printNotFound(std::ostream& out);

    // 등록 완료
    static void printRegistered(const Sample& s, std::ostream& out);
};
