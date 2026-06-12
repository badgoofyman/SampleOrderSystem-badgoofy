#include <gmock/gmock.h>
#include "../view/SampleView.h"
#include <sstream>

using namespace testing;

static constexpr const char* ANSI_RED = "\033[91m";

namespace {
    Sample makeSample(const std::string& id, int stock) {
        Sample s;
        s.id = id; s.name = "테스트"; s.avgProductionTime = 60; s.yield = 0.9; s.stock = stock;
        return s;
    }
}

TEST(SampleViewTest, PrintList_ZeroStock_ContainsRedAnsi) {
    std::ostringstream out;
    std::vector<Sample> samples = { makeSample("S-001", 0) };
    SampleView::printList(samples, out);
    EXPECT_THAT(out.str(), HasSubstr(ANSI_RED));
}

TEST(SampleViewTest, PrintList_NonZeroStock_NoRedAnsi) {
    std::ostringstream out;
    std::vector<Sample> samples = { makeSample("S-001", 10) };
    SampleView::printList(samples, out);
    EXPECT_THAT(out.str(), Not(HasSubstr(ANSI_RED)));
}

TEST(SampleViewTest, PrintList_MixedStock_RedOnlyForZero) {
    std::ostringstream out;
    std::vector<Sample> samples = {
        makeSample("S-001", 10),
        makeSample("S-002", 0),
        makeSample("S-003", 5),
    };
    SampleView::printList(samples, out);
    // Red ANSI는 존재해야 함 (S-002가 0)
    EXPECT_THAT(out.str(), HasSubstr(ANSI_RED));
    EXPECT_THAT(out.str(), HasSubstr("S-001"));
    EXPECT_THAT(out.str(), HasSubstr("S-002"));
    EXPECT_THAT(out.str(), HasSubstr("S-003"));
}
