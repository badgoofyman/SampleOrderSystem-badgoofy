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

    Sample makeSample(const std::string& id, const std::string& name = "테스트") {
        Sample s;
        s.id = id; s.name = name; s.avgProductionTime = 60; s.yield = 0.9; s.stock = 10;
        return s;
    }
};

// ----- 목록 조회 -----

TEST_F(SampleControllerTest, ListSamples_CallsFindAll) {
    Sample s = makeSample("S-001", "웨이퍼");
    s.avgProductionTime = 120; s.yield = 0.92; s.stock = 50;
    EXPECT_CALL(sampleRepo, findAll()).WillOnce(Return(std::vector<Sample>{ s }));

    setup("2\n0\n");
    ctrl->manageSamples();

    EXPECT_THAT(out.str(), HasSubstr("S-001"));
}

// ----- 시료 등록 -----

TEST_F(SampleControllerTest, RegisterSample_CallsSave) {
    EXPECT_CALL(sampleRepo, findById("S-002")).WillOnce(Return(std::nullopt));
    EXPECT_CALL(sampleRepo, save(_)).Times(1);

    setup("1\nS-002\n테스트 시료\n60\n0.85\n10\n0\n");
    ctrl->manageSamples();
}

TEST_F(SampleControllerTest, RegisterSample_DuplicateId_Reprompts) {
    Sample existing = makeSample("S-001");
    EXPECT_CALL(sampleRepo, findById("S-001")).WillOnce(Return(existing));
    EXPECT_CALL(sampleRepo, findById("S-002")).WillOnce(Return(std::nullopt));
    EXPECT_CALL(sampleRepo, save(_)).Times(1);

    // S-001 중복 → 오류 → S-002 정상 등록
    setup("1\nS-001\nS-002\n새 시료\n60\n0.9\n5\n0\n");
    ctrl->manageSamples();

    EXPECT_THAT(out.str(), HasSubstr("이미 존재하는 시료"));
}

TEST_F(SampleControllerTest, RegisterSample_InvalidYield_Reprompts) {
    EXPECT_CALL(sampleRepo, findById("S-003")).WillOnce(Return(std::nullopt));
    EXPECT_CALL(sampleRepo, save(_)).Times(1);

    // 수율 1.5 → 오류 → 0.9 정상 등록
    setup("1\nS-003\n새 시료\n60\n1.5\n0.9\n5\n0\n");
    ctrl->manageSamples();

    EXPECT_THAT(out.str(), HasSubstr("수율"));
}

// ----- 이름 검색 -----

TEST_F(SampleControllerTest, SearchByName_FindsMatch) {
    Sample s = makeSample("S-001", "실리콘 웨이퍼");
    s.avgProductionTime = 120; s.yield = 0.92; s.stock = 50;
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

TEST_F(SampleControllerTest, SearchByName_CaseInsensitive_FindsMatch) {
    Sample s = makeSample("S-001", "Silicon Wafer");
    EXPECT_CALL(sampleRepo, findAll()).WillRepeatedly(Return(std::vector<Sample>{ s }));

    setup("3\nsilicon\n0\n");  // 소문자로 검색
    ctrl->manageSamples();

    EXPECT_THAT(out.str(), HasSubstr("S-001"));
}

// ----- 모니터링 -----

TEST_F(SampleControllerTest, ShowMonitoring_AggregatesStockStatus) {
    Sample s = makeSample("S-001", "웨이퍼");
    s.avgProductionTime = 120; s.yield = 0.92; s.stock = 30;

    Order o;
    o.orderNo = "ORD-001"; o.sampleId = "S-001"; o.quantity = 20;

    EXPECT_CALL(sampleRepo, findAll()).WillOnce(Return(std::vector<Sample>{ s }));
    EXPECT_CALL(orderRepo, findAll()).WillOnce(Return(std::vector<Order>{ o }));

    setup("");
    ctrl->showMonitoring(orderRepo);

    EXPECT_THAT(out.str(), HasSubstr("S-001"));
    EXPECT_THAT(out.str(), HasSubstr("여유"));
}
