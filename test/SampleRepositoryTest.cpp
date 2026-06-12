#include <gmock/gmock.h>
#include <cstdio>
#include <filesystem>
#include "../repository/SampleRepository.h"

namespace fs = std::filesystem;
static const std::string TEST_FILE = "data/test_samples_tmp.json";

class SampleRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        fs::create_directories("data");
        ::remove(TEST_FILE.c_str());
    }
    void TearDown() override {
        ::remove(TEST_FILE.c_str());
    }
    SampleRepository repo{TEST_FILE};

    static Sample makeSample(const std::string& id, int stock = 10) {
        Sample s;
        s.id                = id;
        s.name              = "Test-" + id;
        s.avgProductionTime = 60;
        s.yield             = 0.9;
        s.stock             = stock;
        return s;
    }
};

TEST_F(SampleRepositoryTest, FindAll_emptyFile_returnsEmptyVector) {
    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(SampleRepositoryTest, Save_thenFindAll_returnsSavedSample) {
    repo.save(makeSample("S-001"));
    auto all = repo.findAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id, "S-001");
    EXPECT_EQ(all[0].name, "Test-S-001");
    EXPECT_EQ(all[0].avgProductionTime, 60);
    EXPECT_DOUBLE_EQ(all[0].yield, 0.9);
    EXPECT_EQ(all[0].stock, 10);
}

TEST_F(SampleRepositoryTest, FindById_exists_returnsSample) {
    repo.save(makeSample("S-001"));
    auto result = repo.findById("S-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, "S-001");
}

TEST_F(SampleRepositoryTest, FindById_notExists_returnsNullopt) {
    auto result = repo.findById("NONE");
    EXPECT_FALSE(result.has_value());
}

TEST_F(SampleRepositoryTest, Update_thenFindById_returnsUpdatedValue) {
    repo.save(makeSample("S-001", 10));
    Sample updated = makeSample("S-001", 99);
    repo.update(updated);
    auto result = repo.findById("S-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->stock, 99);
}

TEST_F(SampleRepositoryTest, Remove_thenFindById_returnsNullopt) {
    repo.save(makeSample("S-001"));
    repo.remove("S-001");
    EXPECT_FALSE(repo.findById("S-001").has_value());
}

TEST_F(SampleRepositoryTest, Save_multiple_findAll_returnsAll) {
    repo.save(makeSample("S-001"));
    repo.save(makeSample("S-002"));
    repo.save(makeSample("S-003"));
    EXPECT_EQ(repo.findAll().size(), 3u);
}

TEST_F(SampleRepositoryTest, Persistence_reloadFromFile_sameData) {
    repo.save(makeSample("S-001", 50));
    // 새 인스턴스로 같은 파일 읽기
    SampleRepository repo2(TEST_FILE);
    auto result = repo2.findById("S-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->stock, 50);
}
