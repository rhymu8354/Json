/**
 * @file JsonTests.cpp
 *
 * This module contains the unit tests of the
 * Json::Json class.
 *
 * © 2018 by Richard Walters
 */

#include <gtest/gtest.h>
#include <Json/Json.hpp>

TEST(JsonTests, FromNull) {
    Json::Json json(nullptr);
    ASSERT_EQ("null", json.ToString());
}

TEST(JsonTests, ToNull) {
    const auto json = Json::Json::FromString("null");
    ASSERT_TRUE(json == nullptr);
}

TEST(JsonTests, FromBoolean) {
    Json::Json jsonTrue(true), jsonFalse(false);
    ASSERT_EQ("true", jsonTrue.ToString());
    ASSERT_EQ("false", jsonFalse.ToString());
}

TEST(JsonTests, ToBoolean) {
    const auto jsonTrue = Json::Json::FromString("true");
    const auto jsonFalse = Json::Json::FromString("false");
    ASSERT_TRUE(jsonTrue == true);
    ASSERT_TRUE(jsonFalse == false);
}
