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
    ASSERT_TRUE(jsonTrue == Json::Json(true));
    ASSERT_TRUE((bool)jsonTrue);
    ASSERT_TRUE(jsonFalse == Json::Json(false));
    ASSERT_FALSE((bool)jsonFalse);
}

TEST(JsonTests, NotBooleanDowncastToBoolean) {
    EXPECT_EQ(false, (bool)Json::Json(nullptr));
    EXPECT_EQ(false, (bool)Json::Json(std::string("")));
}

TEST(JsonTests, NotStringDowncastToString) {
    EXPECT_EQ(std::string(""), (std::string)Json::Json(nullptr));
    EXPECT_EQ(std::string(""), (std::string)Json::Json(false));
    EXPECT_EQ(std::string(""), (std::string)Json::Json(true));
}

TEST(JsonTests, FromCString) {
    Json::Json json("Hello, World!");
    ASSERT_EQ("\"Hello, World!\"", json.ToString());
}

TEST(JsonTests, ToCString) {
    const auto json = Json::Json::FromString("\"Hello, World!\"");
    ASSERT_TRUE(json == "Hello, World!");
}

TEST(JsonTests, FromCPlusPlusString) {
    Json::Json json(std::string("Hello, World!"));
    ASSERT_EQ("\"Hello, World!\"", json.ToString());
}

TEST(JsonTests, ToCPlusPlusString) {
    const auto json = Json::Json::FromString("\"Hello, World!\"");
    ASSERT_TRUE(json == std::string("Hello, World!"));
}

TEST(JsonTests, ProperlyEscapedCharactersInString) {
    Json::Json json(std::string("These need to be escaped: \", \\, \b, \f, \n, \r, \t"));
    ASSERT_EQ("\"These need to be escaped: \\\", \\\\, \\b, \\f, \\n, \\r, \\t\"", json.ToString());
}

TEST(JsonTests, ProperlyEscapedUnicodeCharacter) {
    std::string testStringDecoded("This is the Greek word 'kosme': κόσμε");
    std::string testStringEncodedDefault("\"This is the Greek word 'kosme': κόσμε\"");
    std::string testStringEncodedEscapeNonAscii("\"This is the Greek word 'kosme': \\u03BA\\u1F79\\u03C3\\u03BC\\u03B5\"");
    Json::Json json(testStringDecoded);
    auto jsonEncoding = json.ToString();
    EXPECT_EQ(testStringEncodedDefault, jsonEncoding);
    Json::EncodingOptions options;
    options.escapeNonAscii = true;
    jsonEncoding = json.ToString(options);
    EXPECT_EQ(testStringEncodedEscapeNonAscii, jsonEncoding);
    json = Json::Json::FromString(testStringEncodedDefault);
    EXPECT_EQ(testStringDecoded, (std::string)json);
    json = Json::Json::FromString(testStringEncodedEscapeNonAscii);
    EXPECT_EQ(testStringDecoded, (std::string)json);
}

TEST(JsonTests, BadlyEscapedCharacters) {
    auto json = Json::Json::FromString("\"This is bad: \\u123X\"");
    EXPECT_EQ("This is bad: \\u123X", (std::string)json);
    json = Json::Json::FromString("\"This is bad: \\x\"");
    EXPECT_EQ("This is bad: \\x", (std::string)json);
}
