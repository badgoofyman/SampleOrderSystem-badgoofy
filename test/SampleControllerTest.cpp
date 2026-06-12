#include <gmock/gmock.h>
#include "MockRepositories.h"
#include "../controller/SampleController.h"
#include <sstream>

using namespace testing;

class SampleControllerTest : public Test {
protected:
    MockSampleRepository sampleRepo;
    MockOrderRepository  orderRepo;

    std::unique_ptr<SampleController> ctrl;

    void setup(const std::string& input) {
        in.str(input);
        in.clear();
        ctrl = std::make_unique<SampleController>(sampleRepo, in, out);
    }

    std::istringstream in;
    std::ostringstream out;
};

TEST_F(SampleControllerTest, ListSamples_CallsFindAll) {
    Sample s;
    s.id = "S-001"; s.name = "웨이퍼"; s.avgProductionTime = 120; s.yield = 0.92; s.stock = 50;
    EXPECT_CALL(sampleRepo, findAll()).WillOnce(Return(std::vector<Sample>{ s }));

    setup("2\n0\n");  // [2] 목록 조회 → [0] 돌아가기
    ctrl->manageSamples();

    EXPECT_THAT(out.str(), HasSubstr("S-001"));
}

TEST_F(SampleControllerTest, RegisterSample_CallsSave) {
    EXPECT_CALL(sampleRepo, findAll()).WillRepeatedly(Return(std::vector<Sample>{}));
    EXPECT_CALL(sampleRepo, save(_)).Times(1);

    setup("1\nS-002\n테스트 시료\n60\n0.85\n10\n0\n");
    ctrl->manageSamples();
}

TEST_F(SampleControllerTest, SearchByName_FindsMatch) {
    Sample s;
    s.id = "S-001"; s.name = "실리콘 웨이퍼"; s.avgProductionTime = 120; s.yield = 0.92; s.stock = 50;
    EXPECT_CALL(sampleRepo, findAll()).WillRepeatedly(Return(std::vector<Sample>{ s }));

    setup("3\n실리콘\n0\n");
    ctrl->manageSamples();

    EXPECT_THAT(out.str(), HasSubstr("S-001"));
}

TEST_F(SampleControllerTest, SearchByName_PrintsNotFoundWhenNoMatch) {
    EXPECT_CALL(sampleRepo, findAll()).WillRepeatedly(Return(std::vector<Sample>{}));

    setup("3\n없는시료\n0\n");
    ctrl->manageSamples();

    EXPECT_THAT(out.str(), HasSubstr("없습니다"));
}

TEST_F(SampleControllerTest, ShowMonitoring_AggregatesStockStatus) {
    Sample s;
    s.id = "S-001"; s.name = "웨이퍼"; s.avgProductionTime = 120; s.yield = 0.92; s.stock = 30;

    Order o;
    o.orderNo = "ORD-001"; o.sampleId = "S-001"; o.quantity = 20;

    EXPECT_CALL(sampleRepo, findAll()).WillOnce(Return(std::vector<Sample>{ s }));
    EXPECT_CALL(orderRepo, findAll()).WillOnce(Return(std::vector<Order>{ o }));

    setup("");
    ctrl->showMonitoring(orderRepo);

    EXPECT_THAT(out.str(), HasSubstr("S-001"));
    EXPECT_THAT(out.str(), HasSubstr("여유"));
}
