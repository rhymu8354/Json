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
    ASSERT_EQ("null", json.ToEncoding());
}

TEST(JsonTests, ToNull) {
    const auto json = Json::Json::FromEncoding("null");
    ASSERT_TRUE(json == nullptr);
}

TEST(JsonTests, FromBoolean) {
    Json::Json jsonTrue(true), jsonFalse(false);
    ASSERT_EQ("true", jsonTrue.ToEncoding());
    ASSERT_EQ("false", jsonFalse.ToEncoding());
}

TEST(JsonTests, ToBoolean) {
    const auto jsonTrue = Json::Json::FromEncoding("true");
    const auto jsonFalse = Json::Json::FromEncoding("false");
    ASSERT_TRUE(jsonTrue == Json::Json(true));
    ASSERT_TRUE((bool)jsonTrue);
    ASSERT_TRUE(jsonFalse == Json::Json(false));
    ASSERT_FALSE((bool)jsonFalse);
}

TEST(JsonTests, NotBooleanDowncastToBoolean) {
    EXPECT_EQ(false, (bool)Json::Json(nullptr));
    EXPECT_EQ(false, (bool)Json::Json(std::string("")));
}

TEST(JsonTests, NotStringDowncastToEncoding) {
    EXPECT_EQ(std::string(""), (std::string)Json::Json(nullptr));
    EXPECT_EQ(std::string(""), (std::string)Json::Json(false));
    EXPECT_EQ(std::string(""), (std::string)Json::Json(true));
}

TEST(JsonTests, NotIntegerDowncastToInteger) {
    EXPECT_EQ(0, (int)Json::Json(nullptr));
    EXPECT_EQ(0, (int)Json::Json(false));
    EXPECT_EQ(0, (int)Json::Json(true));
    EXPECT_EQ(0, (int)Json::Json("42"));
    EXPECT_EQ(42, (int)Json::Json(42.0));
    EXPECT_EQ(42, (int)Json::Json(42.5));
}

TEST(JsonTests, BadNumbers) {
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("-"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("+"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("+42"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("0025"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("-0025"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding("99999999999999999999999999999999999999999999999999999999"));
    EXPECT_EQ(Json::Json(), Json::Json::FromEncoding(".5"));
}

TEST(JsonTests, NotFloatingPointDowncastToFloatingPoint) {
    EXPECT_EQ(0.0, (double)Json::Json(nullptr));
    EXPECT_EQ(0.0, (double)Json::Json(false));
    EXPECT_EQ(0.0, (double)Json::Json(true));
    EXPECT_EQ(0.0, (int)Json::Json("42"));
    EXPECT_EQ(42.0, (int)Json::Json(42));
}

TEST(JsonTests, FromCString) {
    Json::Json json("Hello, World!");
    ASSERT_EQ("\"Hello, World!\"", json.ToEncoding());
}

TEST(JsonTests, ToCString) {
    const auto json = Json::Json::FromEncoding("\"Hello, World!\"");
    ASSERT_TRUE(json == "Hello, World!");
}

TEST(JsonTests, FromCPlusPlusString) {
    Json::Json json(std::string("Hello, World!"));
    ASSERT_EQ("\"Hello, World!\"", json.ToEncoding());
}

TEST(JsonTests, ToCPlusPlusString) {
    const auto json = Json::Json::FromEncoding("\"Hello, World!\"");
    ASSERT_TRUE(json == std::string("Hello, World!"));
}

TEST(JsonTests, ProperlyEscapedCharactersInString) {
    Json::Json json(std::string("These need to be escaped: \", \\, \b, \f, \n, \r, \t"));
    ASSERT_EQ("\"These need to be escaped: \\\", \\\\, \\b, \\f, \\n, \\r, \\t\"", json.ToEncoding());
}

TEST(JsonTests, ProperlyEscapedUnicodeCharacter) {
    std::string testStringDecoded("This is the Greek word 'kosme': κόσμε");
    std::string testStringEncodedDefault("\"This is the Greek word 'kosme': κόσμε\"");
    std::string testStringEncodedEscapeNonAscii("\"This is the Greek word 'kosme': \\u03BA\\u1F79\\u03C3\\u03BC\\u03B5\"");
    Json::Json json(testStringDecoded);
    auto jsonEncoding = json.ToEncoding();
    EXPECT_EQ(testStringEncodedDefault, jsonEncoding);
    Json::EncodingOptions options;
    options.escapeNonAscii = true;
    options.reencode = true;
    jsonEncoding = json.ToEncoding(options);
    EXPECT_EQ(testStringEncodedEscapeNonAscii, jsonEncoding);
    json = Json::Json::FromEncoding(testStringEncodedDefault);
    EXPECT_EQ(testStringDecoded, (std::string)json);
    json = Json::Json::FromEncoding(testStringEncodedEscapeNonAscii);
    EXPECT_EQ(testStringDecoded, (std::string)json);
}

TEST(JsonTests, BadlyEscapedCharacters) {
    auto json = Json::Json::FromEncoding("\"This is bad: \\u123X\"");
    EXPECT_EQ(Json::Json(), json);
    json = Json::Json::FromEncoding("\"This is bad: \\x\"");
    EXPECT_EQ(Json::Json(), json);
}

TEST(JsonTests, FromInteger) {
    Json::Json json(42);
    ASSERT_EQ("42", json.ToEncoding());
}

TEST(JsonTests, ToInteger) {
    auto json = Json::Json::FromEncoding("42");
    ASSERT_TRUE(json == Json::Json(42));
    json = Json::Json::FromEncoding("-256");
    ASSERT_TRUE(json == Json::Json(-256));
}

TEST(JsonTests, FromFloatingPoint) {
    Json::Json json(3.14159);
    ASSERT_EQ("3.14159", json.ToEncoding());
}

TEST(JsonTests, ToFloatingPoint) {
    auto json = Json::Json::FromEncoding("3.14159");
    ASSERT_TRUE(json == Json::Json(3.14159));
    json = Json::Json::FromEncoding("-17.03");
    ASSERT_TRUE(json == Json::Json(-17.03));
    json = Json::Json::FromEncoding("5.3e-4");
    ASSERT_TRUE(json == Json::Json(5.3e-4));
    json = Json::Json::FromEncoding("5.012e+12");
    ASSERT_TRUE(json == Json::Json(5.012e+12));
    json = Json::Json::FromEncoding("32E+0");
    ASSERT_TRUE(json == Json::Json(32E+0));
}

TEST(JsonTests, SurrogatePairEncoding) {
    Json::Json json(std::string("This should be encoded as a UTF-16 surrogate pair: 𣎴"));
    Json::EncodingOptions options;
    options.escapeNonAscii = true;
    ASSERT_EQ("\"This should be encoded as a UTF-16 surrogate pair: \\uD84C\\uDFB4\"", json.ToEncoding(options));
    json = Json::Json(std::string("This should be encoded as a UTF-16 surrogate pair: 💩"));
    ASSERT_EQ("\"This should be encoded as a UTF-16 surrogate pair: \\uD83D\\uDCA9\"", json.ToEncoding(options));
}

TEST(JsonTests, SurrogatePairDecoding) {
    std::string encoding("\"This should be encoded as a UTF-16 surrogate pair: \\uD84C\\uDFB4\"");
    ASSERT_EQ("This should be encoded as a UTF-16 surrogate pair: 𣎴", (std::string)Json::Json::FromEncoding(encoding));
    encoding = "\"This should be encoded as a UTF-16 surrogate pair: \\uD83D\\uDCA9\"";
    ASSERT_EQ("This should be encoded as a UTF-16 surrogate pair: 💩", (std::string)Json::Json::FromEncoding(encoding));
}

TEST(JsonTests, EncodingOfInvalidJson) {
    auto json = Json::Json::FromEncoding("\"This is bad: \\u123X\"");
    ASSERT_EQ("(Invalid JSON: \"This is bad: \\u123X\")", json.ToEncoding());
}
