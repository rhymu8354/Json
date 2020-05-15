/**
 * @file ValueTests.cpp
 *
 * This module contains the unit tests of the
 * Json::Value class.
 *
 * © 2018 by Richard Walters
 */

#include <gtest/gtest.h>
#include <Json/Value.hpp>
#include <locale.h>

TEST(ValueTests, FromNull) {
    Json::Value json(nullptr);
    ASSERT_EQ("null", json.ToEncoding());
}

TEST(ValueTests, ToNull) {
    const auto json = Json::Value::FromEncoding("null");
    ASSERT_TRUE(json == nullptr);
}

TEST(ValueTests, FromBoolean) {
    Json::Value jsonTrue(true), jsonFalse(false);
    ASSERT_EQ("true", jsonTrue.ToEncoding());
    ASSERT_EQ("false", jsonFalse.ToEncoding());
}

TEST(ValueTests, ToBoolean) {
    const auto jsonTrue = Json::Value::FromEncoding("true");
    const auto jsonFalse = Json::Value::FromEncoding("false");
    ASSERT_TRUE(jsonTrue == Json::Value(true));
    ASSERT_TRUE((bool)jsonTrue);
    ASSERT_TRUE(jsonFalse == Json::Value(false));
    ASSERT_FALSE((bool)jsonFalse);
}

TEST(ValueTests, NotBooleanDowncastToBoolean) {
    EXPECT_EQ(false, (bool)Json::Value(nullptr));
    EXPECT_EQ(false, (bool)Json::Value(std::string("")));
}

TEST(ValueTests, NotStringDowncastToEncoding) {
    EXPECT_EQ(std::string(""), (std::string)Json::Value(nullptr));
    EXPECT_EQ(std::string(""), (std::string)Json::Value(false));
    EXPECT_EQ(std::string(""), (std::string)Json::Value(true));
}

TEST(ValueTests, NotIntegerDowncastToInteger) {
    EXPECT_EQ(0, (int)Json::Value(nullptr));
    EXPECT_EQ(0, (int)Json::Value(false));
    EXPECT_EQ(0, (int)Json::Value(true));
    EXPECT_EQ(0, (int)Json::Value("42"));
    EXPECT_EQ(42, (int)Json::Value(42.0));
    EXPECT_EQ(42, (int)Json::Value(42.5));
}

TEST(ValueTests, BadNumbers) {
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("-"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("+"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("+42"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("0025"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("-0025"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("99999999999999999999999999999999999999999999999999999999"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding(".5"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("1e"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("99999999999999999999999999999999999999999999999999999999999.0"));
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("1e99999999999999999999999999999999999999999999999999999999999"));
}

TEST(ValueTests, NotFloatingPointDowncastToFloatingPoint) {
    EXPECT_EQ(0.0, (double)Json::Value(nullptr));
    EXPECT_EQ(0.0, (double)Json::Value(false));
    EXPECT_EQ(0.0, (double)Json::Value(true));
    EXPECT_EQ(0.0, (int)Json::Value("42"));
    EXPECT_EQ(42.0, (int)Json::Value(42));
}

TEST(ValueTests, FromCString) {
    Json::Value json("Hello, World!");
    ASSERT_EQ("\"Hello, World!\"", json.ToEncoding());
}

TEST(ValueTests, ToCString) {
    const auto json = Json::Value::FromEncoding("\"Hello, World!\"");
    ASSERT_TRUE(json == "Hello, World!");
}

TEST(ValueTests, FromCPlusPlusString) {
    Json::Value json(std::string("Hello, World!"));
    ASSERT_EQ("\"Hello, World!\"", json.ToEncoding());
}

TEST(ValueTests, ToCPlusPlusString) {
    const auto json = Json::Value::FromEncoding("\"Hello, World!\"");
    ASSERT_TRUE(json == std::string("Hello, World!"));
}

TEST(ValueTests, ProperlyEscapedCharactersInString) {
    Json::Value json(std::string("These need to be escaped: \", \\, \b, \f, \n, \r, \t"));
    ASSERT_EQ("\"These need to be escaped: \\\", \\\\, \\b, \\f, \\n, \\r, \\t\"", json.ToEncoding());
}

TEST(ValueTests, ProperlyEscapedUnicodeCharacter) {
    std::string testStringDecoded("This is the Greek word 'kosme': κόσμε");
    std::string testStringEncodedDefault("\"This is the Greek word 'kosme': κόσμε\"");
    std::string testStringEncodedEscapeNonAscii("\"This is the Greek word 'kosme': \\u03BA\\u1F79\\u03C3\\u03BC\\u03B5\"");
    Json::Value json(testStringDecoded);
    auto jsonEncoding = json.ToEncoding();
    EXPECT_EQ(testStringEncodedDefault, jsonEncoding);
    Json::EncodingOptions options;
    options.escapeNonAscii = true;
    options.reencode = true;
    jsonEncoding = json.ToEncoding(options);
    EXPECT_EQ(testStringEncodedEscapeNonAscii, jsonEncoding);
    json = Json::Value::FromEncoding(testStringEncodedDefault);
    EXPECT_EQ(testStringDecoded, (std::string)json);
    json = Json::Value::FromEncoding(testStringEncodedEscapeNonAscii);
    EXPECT_EQ(testStringDecoded, (std::string)json);
}

TEST(ValueTests, BadlyEscapedCharacters) {
    auto json = Json::Value::FromEncoding("\"This is bad: \\u123X\"");
    EXPECT_EQ(Json::Value(), json);
    json = Json::Value::FromEncoding("\"This is bad: \\x\"");
    EXPECT_EQ(Json::Value(), json);
}

TEST(ValueTests, FromInteger) {
    Json::Value json(42);
    ASSERT_EQ("42", json.ToEncoding());
}

TEST(ValueTests, ToInteger) {
    auto json = Json::Value(42);
    EXPECT_EQ(42, (int)json);
    json = Json::Value(-256);
    EXPECT_EQ(-256, (int)json);
}

TEST(ValueTests, FromSize) {
    Json::Value json((size_t)42);
    ASSERT_EQ("42", json.ToEncoding());
}

TEST(ValueTests, ToSize) {
    auto json = Json::Value((size_t)42);
    EXPECT_EQ((size_t)42, (size_t)json);
}

TEST(ValueTests, FromIntMax) {
    Json::Value json((intmax_t)42);
    ASSERT_EQ("42", json.ToEncoding());
}

TEST(ValueTests, ToIntMax) {
    auto json = Json::Value((intmax_t)42);
    EXPECT_EQ((intmax_t)42, (intmax_t)json);
}

TEST(ValueTests, FromFloatingPoint) {
    Json::Value json(0.0056);
    EXPECT_EQ("0.0056", json.ToEncoding());
    json = Json::Value(12300000.78);
    EXPECT_EQ("12300000.78", json.ToEncoding());
    json = Json::Value(3.14159);
    EXPECT_EQ("3.14159", json.ToEncoding());
    json = Json::Value(0.0);
    EXPECT_EQ("0.0", json.ToEncoding());
    json = Json::Value(123.0);
    EXPECT_EQ("123.0", json.ToEncoding());
    json = Json::Value(60412.769);
    EXPECT_EQ("60412.769", json.ToEncoding());
    json = Json::Value(604124.769);
    EXPECT_EQ("604124.769", json.ToEncoding());
}

TEST(ValueTests, FromFloatingPoint_Danish_Locale) {
    const auto originalLocale = (std::string)setlocale(LC_ALL, NULL);
    (void)setlocale(LC_ALL, "de_DE");
    Json::Value json(3.14159);
    EXPECT_EQ("3.14159", json.ToEncoding());
    (void)setlocale(LC_ALL, originalLocale.c_str());
}

TEST(ValueTests, ToFloatingPoint) {
    auto json = Json::Value(3.14159);
    EXPECT_EQ(3.14159, (double)json);
    json = Json::Value(-17.03);
    EXPECT_EQ(-17.03, (double)json);
    json = Json::Value(5.3e-4);
    EXPECT_EQ(5.3e-4, (double)json);
    json = Json::Value(5.012e+12);
    EXPECT_EQ(5.012e+12, (double)json);
    json = Json::Value(32.0);
    EXPECT_EQ(32.0, (double)json);
    json = Json::Value(0.0);
    EXPECT_EQ(0.0, (double)json);
}

TEST(ValueTests, SurrogatePairEncoding) {
    Json::Value json(std::string("This should be encoded as a UTF-16 surrogate pair: 𣎴"));
    Json::EncodingOptions options;
    options.escapeNonAscii = true;
    ASSERT_EQ("\"This should be encoded as a UTF-16 surrogate pair: \\uD84C\\uDFB4\"", json.ToEncoding(options));
    json = Json::Value(std::string("This should be encoded as a UTF-16 surrogate pair: 💩"));
    ASSERT_EQ("\"This should be encoded as a UTF-16 surrogate pair: \\uD83D\\uDCA9\"", json.ToEncoding(options));
}

TEST(ValueTests, ValidSurrogatePairDecoding) {
    std::string encoding("\"This should be encoded as a UTF-16 surrogate pair: \\uD84C\\uDFB4\"");
    ASSERT_EQ("This should be encoded as a UTF-16 surrogate pair: 𣎴", (std::string)Json::Value::FromEncoding(encoding));
    encoding = "\"This should be encoded as a UTF-16 surrogate pair: \\uD83D\\uDCA9\"";
    ASSERT_EQ("This should be encoded as a UTF-16 surrogate pair: 💩", (std::string)Json::Value::FromEncoding(encoding));
}

TEST(ValueTests, IncompleteSurrogatePairDecoding) {
    auto json = Json::Value::FromEncoding("\"This is bad: \\ud834\"");
    ASSERT_EQ("(Invalid JSON: \"This is bad: \\ud834\")", json.ToEncoding());
    json = Json::Value::FromEncoding("\"This is also bad: \\ud834 yo!\"");
    ASSERT_EQ("(Invalid JSON: \"This is also bad: \\ud834 yo!\")", json.ToEncoding());
}

TEST(ValueTests, EncodingOfInvalidJson) {
    auto json = Json::Value::FromEncoding("\"This is bad: \\u123X\"");
    ASSERT_EQ("(Invalid JSON: \"This is bad: \\u123X\")", json.ToEncoding());
}

TEST(ValueTests, DecodeArrayNoWhitespace) {
    const std::string encoding("[1,\"Hello\",true]");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Array, json.GetType());
    ASSERT_EQ(3, json.GetSize());
    EXPECT_EQ(Json::Value::Type::Integer, json[0].GetType());
    EXPECT_EQ(1, (int)json[0]);
    EXPECT_EQ(Json::Value::Type::String, json[1].GetType());
    EXPECT_EQ("Hello", (std::string)json[1]);
    EXPECT_EQ(Json::Value::Type::Boolean, json[2].GetType());
    EXPECT_EQ(true, (bool)json[2]);
    EXPECT_TRUE(json[3] == nullptr);
}

TEST(ValueTests, DecodeArrayWithWhitespace) {
    const std::string encoding(" [ 1 ,\r \t \"Hello\" \r\n ,\n true   ]  ");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Array, json.GetType());
    ASSERT_EQ(3, json.GetSize());
    EXPECT_EQ(Json::Value::Type::Integer, json[0].GetType());
    EXPECT_EQ(1, (int)json[0]);
    EXPECT_EQ(Json::Value::Type::String, json[1].GetType());
    EXPECT_EQ("Hello", (std::string)json[1]);
    EXPECT_EQ(Json::Value::Type::Boolean, json[2].GetType());
    EXPECT_EQ(true, (bool)json[2]);
}

TEST(ValueTests, DecodeArraysWithinArrays) {
    const std::string encoding("[1,[2,3],4,[5,6,7]]");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Array, json.GetType());
    ASSERT_EQ(4, json.GetSize());
    EXPECT_EQ(Json::Value::Type::Integer, json[0].GetType());
    EXPECT_EQ(1, (int)json[0]);
    EXPECT_EQ(Json::Value::Type::Array, json[1].GetType());
    ASSERT_EQ(2, json[1].GetSize());
    EXPECT_EQ(Json::Value::Type::Integer, json[1][0].GetType());
    EXPECT_EQ(2, (int)json[1][0]);
    EXPECT_EQ(Json::Value::Type::Integer, json[1][1].GetType());
    EXPECT_EQ(3, (int)json[1][1]);
    EXPECT_EQ(Json::Value::Type::Integer, json[2].GetType());
    EXPECT_EQ(4, (int)json[2]);
    EXPECT_EQ(Json::Value::Type::Array, json[3].GetType());
    ASSERT_EQ(3, json[3].GetSize());
    EXPECT_EQ(Json::Value::Type::Integer, json[3][0].GetType());
    EXPECT_EQ(5, (int)json[3][0]);
    EXPECT_EQ(Json::Value::Type::Integer, json[3][1].GetType());
    EXPECT_EQ(6, (int)json[3][1]);
    EXPECT_EQ(Json::Value::Type::Integer, json[3][2].GetType());
    EXPECT_EQ(7, (int)json[3][2]);
}

TEST(ValueTests, DecodeUnterminatedOuterArray) {
    const std::string encoding("[1,\"Hello\",true");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Invalid, json.GetType());
}

TEST(ValueTests, DecodeUnterminatedInnerArray) {
    const std::string encoding("{ \"value\": 1, \"array\": [42, 57, \"flag\": true }");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Invalid, json.GetType());
}

TEST(ValueTests, DecodeUnterminatedInnerString) {
    const std::string encoding("[1,\"Hello,true]");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Invalid, json.GetType());
}

TEST(ValueTests, DecodeObject) {
    const std::string encoding("{\"value\": 42, \"\": \"Pepe\", \"the handles\":[3,7], \"is,live\": true}");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Object, json.GetType());
    ASSERT_EQ(4, json.GetSize());
    EXPECT_TRUE(json.Has("value"));
    EXPECT_TRUE(json.Has(""));
    EXPECT_TRUE(json.Has("the handles"));
    EXPECT_TRUE(json.Has("is,live"));
    EXPECT_FALSE(json.Has("FeelsBadMan"));
    const auto value = json["value"];
    EXPECT_EQ(Json::Value::Type::Integer, value.GetType());
    EXPECT_EQ(42, (int)value);
    const auto empty = json[""];
    EXPECT_EQ(Json::Value::Type::String, empty.GetType());
    EXPECT_EQ("Pepe", (std::string)empty);
    const auto theHandles = json["the handles"];
    EXPECT_EQ(Json::Value::Type::Array, theHandles.GetType());
    ASSERT_EQ(2, theHandles.GetSize());
    EXPECT_EQ(Json::Value::Type::Integer, theHandles[0].GetType());
    ASSERT_EQ(3, (int)theHandles[0]);
    EXPECT_EQ(Json::Value::Type::Integer, theHandles[1].GetType());
    ASSERT_EQ(7, (int)theHandles[1]);
    const auto isLive = json["is,live"];
    EXPECT_EQ(Json::Value::Type::Boolean, isLive.GetType());
    EXPECT_EQ(true, (bool)isLive);
}

TEST(ValueTests, NumericIndexNotArray) {
    const Json::Value json(42);
    ASSERT_TRUE(json[0] == nullptr);
}

TEST(ValueTests, BuildAndEncodeArray) {
    Json::Value json(Json::Value::Type::Array);
    auto& first = json.Add(43);
    EXPECT_EQ(43, (int)first);
    first = 42;
    auto& newFirst = json.Insert("Hello!", 0);
    EXPECT_EQ("Hello!", (std::string)newFirst);
    newFirst = "Hello";
    json.Add(3);
    json.Insert("World", 1);
    ASSERT_EQ("[\"Hello\",\"World\",42,3]", json.ToEncoding());
    json.Remove(1);
    ASSERT_EQ("[\"Hello\",42,3]", json.ToEncoding());
    json.Remove(0);
    ASSERT_EQ("[42,3]", json.ToEncoding());
}

TEST(ValueTests, BuildAndEncodeObject) {
    Json::Value json(Json::Value::Type::Object);
    auto& answer = json.Set("answer", 43);
    EXPECT_EQ(43, (int)answer);
    answer = 42;
    json.Set("Hello", 0);
    json.Set("Hello", "World");
    json.Set("PogChamp", true);
    json.Set("Don't look here", nullptr);
    ASSERT_EQ("{\"Don't look here\":null,\"Hello\":\"World\",\"PogChamp\":true,\"answer\":42}", json.ToEncoding());
    json.Remove("answer");
    ASSERT_EQ("{\"Don't look here\":null,\"Hello\":\"World\",\"PogChamp\":true}", json.ToEncoding());
}

TEST(ValueTests, CompareArrays) {
    const auto json1 = Json::Value::FromEncoding("[42, 7]");
    const auto json2 = Json::Value::FromEncoding(" [42,7] ");
    const auto json3 = Json::Value::FromEncoding(" [43,7] ");
    EXPECT_EQ(json1, json2);
    EXPECT_NE(json1, json3);
    EXPECT_NE(json2, json3);
}

TEST(ValueTests, CompareObjects) {
    const auto json1 = Json::Value::FromEncoding("{\"answer\":42}");
    const auto json2 = Json::Value::FromEncoding("{\"answer\" :  42 }");
    const auto json3 = Json::Value::FromEncoding("{\"answer\":19}");
    const auto json4 = Json::Value::FromEncoding("{\"answer\":42, \"nothing\": null}");
    const auto json5 = Json::Value::FromEncoding("{\"xyz\":42, \"123\": null}");
    EXPECT_EQ(json1, json2);
    EXPECT_NE(json1, json3);
    EXPECT_NE(json2, json3);
    EXPECT_NE(json1, json4);
    EXPECT_NE(json5, json4);
}

TEST(ValueTests, AddObjectToItselfCopy) {
    Json::Value json(Json::Value::Type::Array);
    json.Add(42);
    json.Add(json);
    EXPECT_EQ("[42,[42]]", json.ToEncoding());
}

TEST(ValueTests, AddObjectToItselfMove) {
    Json::Value json(Json::Value::Type::Array);
    json.Add(42);
    json.Add(std::move(json));
    EXPECT_EQ("[42,[42]]", json.ToEncoding());
}

TEST(ValueTests, ReassignValue) {
    Json::Value json1(42);
    Json::Value json2(Json::Value::Type::Array);
    json2.Add(42);
    json2.Add("Hello");
    json1 = json2;
    json1.Add(false);
    json2.Remove(0);
    json2.Add(true);
    EXPECT_EQ("[42,\"Hello\",false]", json1.ToEncoding());
    EXPECT_EQ("[\"Hello\",true]", json2.ToEncoding());
}

TEST(ValueTests, PrettyPrintingObject) {
    const std::string encoding("{\"value\": 42, \"\": \"Pepe\", \"the handles\":[3,7], \"is,live\": true}");
    const auto json = Json::Value::FromEncoding(encoding);
    Json::EncodingOptions options;
    options.reencode = true;
    options.pretty = true;
    options.spacesPerIndentationLevel = 4;
    options.wrapThreshold = 30;
    ASSERT_EQ(
        (
            "{\r\n"
            "    \"\": \"Pepe\",\r\n"
            "    \"is,live\": true,\r\n"
            "    \"the handles\": [3, 7],\r\n"
            "    \"value\": 42\r\n"
            "}"
        ),
        json.ToEncoding(options)
    );
}

TEST(ValueTests, PrettyPrintingArray) {
    const std::string encoding("[1,[2,3],4,[5,6,7]]");
    const auto json = Json::Value::FromEncoding(encoding);
    Json::EncodingOptions options;
    options.reencode = true;
    options.pretty = true;
    options.spacesPerIndentationLevel = 4;
    options.wrapThreshold = 10;
    ASSERT_EQ(
        (
            "[\r\n"
            "    1,\r\n"
            "    [2, 3],\r\n"
            "    4,\r\n"
            "    [\r\n"
            "        5,\r\n"
            "        6,\r\n"
            "        7\r\n"
            "    ]\r\n"
            "]"
        ),
        json.ToEncoding(options)
    );
}

TEST(ValueTests, JsonArrayInitializerList) {
    const auto json = Json::Array({
        42, "Hello, World!", true
    });
    ASSERT_EQ("[42,\"Hello, World!\",true]", json.ToEncoding());
}

TEST(ValueTests, JsonObjectInitializerList) {
    const auto json = Json::Object({
        {"Answer", 42},
        {"Greeting", "Hello, World!"},
        {"List", Json::Array({1, 2, 3})},
    });
    ASSERT_EQ("{\"Answer\":42,\"Greeting\":\"Hello, World!\",\"List\":[1,2,3]}", json.ToEncoding());
}

TEST(ValueTests, JsonDecodeObjectWithDuplicateKeys) {
    const auto json = Json::Value::FromEncoding("{\"key\": 3, \"key\": true}");
    ASSERT_EQ(Json::Value::Type::Object, json.GetType());
    ASSERT_EQ(1, json.GetSize());
    ASSERT_EQ(Json::Value::Type::Boolean, json["key"].GetType());
    Json::EncodingOptions options;
    options.reencode = true;
    ASSERT_EQ("{\"key\":true}", json.ToEncoding(options));
}

TEST(ValueTests, DecodeObjectsWithinObjects) {
    const std::string encoding("{\"nested\":{\"value\":42, \"good\": true}, \"end\": null}");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(Json::Value::Type::Object, json.GetType());
    ASSERT_EQ(2, json.GetSize());
    ASSERT_TRUE(json.Has("nested"));
    ASSERT_EQ(Json::Value::Type::Object, json["nested"].GetType());
    ASSERT_EQ(2, json["nested"].GetSize());
    ASSERT_TRUE(json["nested"].Has("value"));
    ASSERT_EQ(Json::Value::Type::Integer, json["nested"]["value"].GetType());
    ASSERT_EQ(42, (int)json["nested"]["value"]);
    ASSERT_TRUE(json["nested"].Has("good"));
    ASSERT_EQ(Json::Value::Type::Boolean, json["nested"]["good"].GetType());
    ASSERT_EQ(true, (bool)json["nested"]["good"]);
    ASSERT_TRUE(json.Has("end"));
    EXPECT_EQ(Json::Value::Type::Null, json["end"].GetType());
}

TEST(ValueTests, GetKeys) {
    const std::string encoding("{\"value\": 42, \"\": \"Pepe\", \"the handles\":[3,7], \"is,live\": true}");
    const auto json = Json::Value::FromEncoding(encoding);
    ASSERT_EQ(
        (std::vector< std::string >{
            "",
            "is,live",
            "the handles",
            "value",
        }),
        json.GetKeys()
    );
}

TEST(ValueTests, ModifyValueInArray) {
    auto array = Json::Array({1, 2, 3});
    array[1] = 0;
    EXPECT_EQ(
        (Json::Array({1, 0, 3})),
        array
    );
}

TEST(ValueTests, ModifyValueInObject) {
    auto obj = Json::Object({
        {"x", 1},
        {"y", 2},
        {"z", 3},
    });
    obj["y"] = 0;
    EXPECT_EQ(
        (Json::Object({
            {"x", 1},
            {"y", 0},
            {"z", 3},
        })),
        obj
    );
}

TEST(ValueTests, ReferenceValueInArray) {
    auto array = Json::Array({1, 2, 3});
    auto* value = &array[1];
    *value = 0;
    EXPECT_EQ(
        (Json::Array({1, 0, 3})),
        array
    );
}

TEST(ValueTests, ReferenceValueInObject) {
    auto obj = Json::Object({
        {"x", 1},
        {"y", 2},
        {"z", 3},
    });
    auto* value = &obj["y"];
    *value = 0;
    EXPECT_EQ(
        (Json::Object({
            {"x", 1},
            {"y", 0},
            {"z", 3},
        })),
        obj
    );
}

TEST(ValueTests, UndefinedIndexCannotBeModified) {
    Json::Value json(42);
    auto& null = json[0];
    null = 0;
    EXPECT_EQ(Json::Value::Type::Null, null.GetType());
}

TEST(ValueTests, UndefinedIndexCannotBeMoved) {
    Json::Value json(42);
    auto& null = json[0];
    Json::Value tryToMoveNullHere(std::move(null));
    tryToMoveNullHere = 0;
    EXPECT_EQ(Json::Value::Type::Null, json[0].GetType());
}

TEST(ValueTests, ConstArrayIndexOutOfRange) {
    const auto array = Json::Array({1, 2, 3});
    const auto& value = array[10];
    EXPECT_EQ(Json::Value::Type::Null, value.GetType());
}

TEST(ValueTests, ConstArrayNegativeIndex) {
    const auto array = Json::Array({1, 2, 3});
    const auto& value = array[-1];
    EXPECT_EQ(Json::Value::Type::Null, value.GetType());
}

TEST(ValueTests, ConstObjectNameNotFound) {
    const auto obj = Json::Object({
        {"Hello", "World"},
    });
    const auto& value = obj["x"];
    EXPECT_EQ(Json::Value::Type::Null, value.GetType());
}

TEST(ValueTests, ConstObjectNullName) {
    const auto obj = Json::Object({
        {"Hello", "World"},
    });
    const auto& value = obj[nullptr];
    EXPECT_EQ(Json::Value::Type::Null, value.GetType());
}

TEST(ValueTests, MutableArrayIndexOutOfRange) {
    auto array = Json::Array({1, 2, 3});
    auto& value = array[5];
    EXPECT_EQ(Json::Value::Type::Null, value.GetType());
    value = 4;
    EXPECT_EQ(4, (int)value);
    EXPECT_EQ(
        (Json::Array({
            1, 2, 3, nullptr, nullptr, 4
        })),
        array
    );
}

TEST(ValueTests, MutableArrayNegativeIndex) {
    auto array = Json::Array({1, 2, 3});
    auto& value = array[-1];
    EXPECT_EQ(Json::Value::Type::Null, value.GetType());
    value = 4;
    EXPECT_EQ(Json::Value::Type::Null, value.GetType());
    EXPECT_EQ(
        (Json::Array({
            1, 2, 3
        })),
        array
    );
}

TEST(ValueTests, MutableObjectNameNotFound) {
    auto obj = Json::Object({
        {"Hello", "World"},
    });
    auto& value = obj["x"];
    EXPECT_EQ(Json::Value::Type::Null, value.GetType());
    value = 42;
    EXPECT_EQ(42, (int)value);
    EXPECT_EQ(
        (Json::Object({
            {"Hello", "World"},
            {"x", 42},
        })),
        obj
    );
}

TEST(ValueTests, MutableObjectNullName) {
    auto obj = Json::Object({
        {"Hello", "World"},
    });
    auto& value = obj[nullptr];
    EXPECT_EQ(Json::Value::Type::Null, value.GetType());
    value = 42;
    EXPECT_EQ(Json::Value::Type::Null, value.GetType());
    EXPECT_EQ(
        (Json::Object({
            {"Hello", "World"},
        })),
        obj
    );
}

TEST(ValueTests, MovedValueBecomesInvalid) {
    auto obj = Json::Value(42);
    auto newLocation = std::move(obj);
    EXPECT_EQ(Json::Value(42), newLocation);
    EXPECT_EQ(Json::Value::Type::Invalid, obj.GetType());
}

TEST(ValueTests, MoveInsert) {
    auto element = Json::Array({
        1, 1, 2, 3, 5, 8, 13
    });
    const auto elementCopy = element;
    auto array = Json::Array({42, "abc"});
    const auto& movedElement = array.Insert(std::move(element), 1);
    EXPECT_EQ(
        Json::Array({
            42,
            elementCopy,
            "abc",
        }),
        array
    );
    EXPECT_EQ(
        Json::Value::Type::Invalid,
        element.GetType()
    );
    EXPECT_EQ(elementCopy, movedElement);
}

TEST(ValueTests, MoveAdd) {
    auto element = Json::Array({
        1, 1, 2, 3, 5, 8, 13
    });
    const auto elementCopy = element;
    auto array = Json::Array({42, "abc"});
    const auto& movedElement = array.Add(std::move(element));
    EXPECT_EQ(
        Json::Array({
            42,
            "abc",
            elementCopy,
        }),
        array
    );
    EXPECT_EQ(
        Json::Value::Type::Invalid,
        element.GetType()
    );
    EXPECT_EQ(elementCopy, movedElement);
}

TEST(ValueTests, FloatingPointComparisonWorksDespiteRoundingError) {
    const auto value = Json::Value(0.15);
    const auto encoding = value.ToEncoding();
    EXPECT_EQ(value, Json::Value::FromEncoding(encoding));
}

TEST(ValueTests, DecodeObjectWithStringValueContainingEscapedDelimiter) {
    const std::string testString = "These need to be escaped: \", \\, \b, \f, \n, \r, \t";
    const auto testObject = Json::Object({
        {"foo", testString},
    });
    const auto testEncoding = testObject.ToEncoding();
    EXPECT_EQ(
        testObject,
        Json::Value::FromEncoding(testEncoding)
    );
}

TEST(ValueTests, ArrayIteratorProtocol) {
    const auto array = Json::Array({1, 2, 3});
    std::vector< int > values;
    for (auto arrayEntry: array) {
        values.push_back(arrayEntry.value());
    }
    EXPECT_EQ(
        std::vector< int >({1, 2, 3}),
        values
    );
}

TEST(ValueTests, ObjectIteratorProtocol) {
    const auto object = Json::Object({
        {"Answer", 42},
        {"Greeting", "Hello, World!"},
        {"List", Json::Array({1, 2, 3})},
    });
    std::vector< std::string > keys;
    std::vector< Json::Value > values;
    for (auto objectEntry: object) {
        keys.push_back(objectEntry.key());
        values.push_back(objectEntry.value());
    }
    EXPECT_EQ(
        std::vector< std::string >({
            "Answer",
            "Greeting",
            "List"
        }),
        keys
    );
    EXPECT_EQ(
        std::vector< Json::Value >({
            42,
            "Hello, World!",
            Json::Array({1, 2, 3})
        }),
        values
    );
}

TEST(ValueTests, BadEncodings) {
    EXPECT_EQ(Json::Value(), Json::Value::FromEncoding("\""));
}
