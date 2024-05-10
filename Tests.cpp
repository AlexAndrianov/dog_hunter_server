#include "dog_hunter_core/include/model.h"
#include "Database.h"

#include <gtest/gtest.h>


namespace dh {

TEST(ModelTest, WalksSearchSmallRadius) {

    const auto model = ModelData::getDummyModelData();
    const auto walks = model.getWalks(Coordinate{-37.8136, 144.9631}, 500, QDateTime(QDate(2024, 5, 12), QTime(12, 0)));

    EXPECT_EQ(walks.size(), 0);
}

TEST(ModelTest, WalksSearchMediumRadius) {

    const auto model = ModelData::getDummyModelData();
    const auto walks = model.getWalks(Coordinate{-37.8136, 144.9631}, 1000, QDateTime(QDate(2024, 5, 12), QTime(12, 0)));

    EXPECT_EQ(walks.size(), 4);
}

TEST(ModelTest, WalksSearchHightRadius) {

    const auto model = ModelData::getDummyModelData();
    const auto walks = model.getWalks(Coordinate{-37.8136, 144.9631}, 3000, QDateTime(QDate(2024, 5, 12), QTime(12, 0)));

    EXPECT_EQ(walks.size(), 6);
}

TEST(ModelTest, WalksSearchMediumRadiusLate) {

    const auto model = ModelData::getDummyModelData();
    const auto walks = model.getWalks(Coordinate{-37.8136, 144.9631}, 1000, QDateTime(QDate(2024, 5, 12), QTime(12, 50)));

    EXPECT_EQ(walks.size(), 0);
}

TEST(DataBaseTest, loginUseCase) {

    auto &database = Database::getInstance();

    Database::QueryResult res;
    auto dogOwnerPtr = database.getDogOwner("1@gmail.com", "123", res);

    EXPECT_EQ(res, Database::QueryResult::Ok);
    EXPECT_EQ(dogOwnerPtr != nullptr, true);
    EXPECT_EQ(dogOwnerPtr->_email, "1@gmail.com");
    EXPECT_EQ(dogOwnerPtr->_password, "123");
    EXPECT_EQ(dogOwnerPtr->_dogs.begin()->first, "Buddy");
}

}
