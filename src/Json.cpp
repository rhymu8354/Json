/**
 * @file Json.cpp
 *
 * This module contains the implementation of the Json::Json class.
 *
 * © 2018 by Richard Walters
 */

#include <algorithm>
#include <Json/Json.hpp>
#include <map>
#include <math.h>
#include <set>
#include <stack>
#include <string>
#include <SystemAbstractions/StringExtensions.hpp>
#include <Utf8/Utf8.hpp>
#include <vector>

namespace {

    /**
     * These are the character that are considered "whitespace"
     * by the JSON standard (RFC 7159).
     */
    constexpr const char* WHITESPACE_CHARACTERS = " \t\r\n";

    /**
     * This maps the escaped representations of special characters
     * back to the actual characters they represent.
     */
    const std::map< Utf8::UnicodeCodePoint, Utf8::UnicodeCodePoint > SPECIAL_ESCAPE_DECODINGS {
        { 0x22, 0x22 }, // '"'
        { 0x5C, 0x5C }, // '\\'
        { 0x2F, 0x2F }, // '\\'
        { 0x62, 0x08 }, // '\b'
        { 0x66, 0x0C }, // '\f'
        { 0x6E, 0x0A }, // '\n'
        { 0x72, 0x0D }, // '\r'
        { 0x74, 0x09 }, // '\t'
    };

    /**
     * This maps special characters to their escaped representations.
     */
    const std::map< Utf8::UnicodeCodePoint, Utf8::UnicodeCodePoint > SPECIAL_ESCAPE_ENCODINGS{
        { 0x22, 0x22 }, // '"'
        { 0x5C, 0x5C }, // '\\'
        { 0x2F, 0x2F }, // '\\'
        { 0x08, 0x62 }, // '\b'
        { 0x0C, 0x66 }, // '\f'
        { 0x0A, 0x6E }, // '\n'
        { 0x0D, 0x72 }, // '\r'
        { 0x09, 0x74 }, // '\t'
    };

    /**
     * This function returns a string consisting of the four hex digits
     * matching the given code point in hexadecimal.
     *
     * @param[in] cp
     *     This is the code point to render as four hex digits.
     *
     * @return
     *     This is the four hex digit rendering of the given code point.
     */
    std::string CodePointToFourHexDigits(Utf8::UnicodeCodePoint cp) {
        std::string rendering;
        for (size_t i = 0; i < 4; ++i) {
            const auto nibble = ((cp >> (4 * (3 - i))) & 0x0F);
            if (nibble < 10) {
                rendering += (char)nibble + '0';
            } else {
                rendering += (char)(nibble - 10) + 'A';
            }
        }
        return rendering;
    }

    /**
     * This function produces the escaped version of the given string.
     *
     * @param[in] s
     *     This is the string which needs to be escaped.
     *
     * @param[in] options
     *     This is used to configure various options having to do with
     *     encoding a Json object into its string format.
     *
     * @return
     *     The escaped string is returned.
     */
    std::string Escape(
        const std::string& s,
        const Json::EncodingOptions& options
    ) {
        Utf8::Utf8 decoder, encoder;
        std::string output;
        for (const auto cp: decoder.Decode(s)) {
            if (
                (cp == 0x22)
                || (cp == 0x5C)
                || (cp < 0x20)
            ) {
                output += '\\';
                const auto entry = SPECIAL_ESCAPE_ENCODINGS.find(cp);
                if (entry == SPECIAL_ESCAPE_ENCODINGS.end()) {
                    output += 'u';
                    output += CodePointToFourHexDigits(cp);
                } else {
                    output += (char)entry->second;
                }
            } else if (
                options.escapeNonAscii
                && (cp > 0x7F)
            ) {
                if (cp > 0xFFFF) {
                    output += "\\u";
                    output += CodePointToFourHexDigits(0xD800 + (((cp - 0x10000) >> 10) & 0x3FF));
                    output += "\\u";
                    output += CodePointToFourHexDigits(0xDC00 + ((cp - 0x10000) & 0x3FF));
                } else {
                    output += "\\u";
                    output += CodePointToFourHexDigits(cp);
                }
            } else {
                const auto encoding = encoder.Encode({cp});
                output += std::string(
                    encoding.begin(),
                    encoding.end()
                );
            }
        }
        return output;
    }

    /**
     * This function produces the unescaped version of the given string.
     *
     * @param[in] s
     *     This is the string which needs to be unescaped.
     *
     * @param[out] output
     *     This is where to put the unescaped string.
     *
     * @return
     *     An indication of whether or not the input string was a valid
     *     encoding is returned.
     */
    bool Unescape(
        const std::string& s,
        std::string& output
    ) {
        Utf8::Utf8 decoder, encoder;
        size_t state = 0;
        Utf8::UnicodeCodePoint cpFromHexDigits = 0;
        Utf8::UnicodeCodePoint firstHalfOfSurrogatePair = 0;
        std::vector< Utf8::UnicodeCodePoint > hexDigitsOriginal;
        for (const auto cp: decoder.Decode(s)) {
            switch (state) {
                case 0: { // initial state
                    if (cp == 0x5C) { // '\\'
                        state = 1;
                    } else if (firstHalfOfSurrogatePair == 0) {
                        const auto encoding = encoder.Encode({cp});
                        output += std::string(
                            encoding.begin(),
                            encoding.end()
                        );
                    } else {
                        return false;
                    }
                } break;

                case 1: { // escape character
                    if (cp == 0x75) { // 'u'
                        state = 2;
                        cpFromHexDigits = 0;
                        hexDigitsOriginal = { 0x5C, 0x75 };
                    } else if (firstHalfOfSurrogatePair == 0) {
                        Utf8::UnicodeCodePoint alternative = cp;
                        const auto entry = SPECIAL_ESCAPE_DECODINGS.find(cp);
                        if (entry == SPECIAL_ESCAPE_DECODINGS.end()) {
                            return false;
                        }
                        const auto encoding = encoder.Encode({entry->second});
                        output += std::string(
                            encoding.begin(),
                            encoding.end()
                        );
                        state = 0;
                    } else {
                        return false;
                    }
                } break;

                case 2:   // first hexdigit of escape uXXXX
                case 3:   // second hexdigit of escape uXXXX
                case 4:   // third hexdigit of escape uXXXX
                case 5: { // fourth hexdigit of escape uXXXX
                    hexDigitsOriginal.push_back(cp);
                    cpFromHexDigits <<= 4;
                    if (
                        (cp >= (Utf8::UnicodeCodePoint)'0')
                        && (cp <= (Utf8::UnicodeCodePoint)'9')
                    ) {
                        cpFromHexDigits += (cp - (Utf8::UnicodeCodePoint)'0');
                    } else if (
                        (cp >= (Utf8::UnicodeCodePoint)'A')
                        && (cp <= (Utf8::UnicodeCodePoint)'F')
                    ) {
                        cpFromHexDigits += (cp - (Utf8::UnicodeCodePoint)'A' + 10);
                    } else if (
                        (cp >= (Utf8::UnicodeCodePoint)'a')
                        && (cp <= (Utf8::UnicodeCodePoint)'f')
                    ) {
                        cpFromHexDigits += (cp - (Utf8::UnicodeCodePoint)'a' + 10);
                    } else {
                        return false;
                    }
                    if (++state == 6) {
                        state = 0;
                        if (
                            (cpFromHexDigits >= 0xD800)
                            && (cpFromHexDigits <= 0xDFFF)
                        ) {
                            if (firstHalfOfSurrogatePair == 0) {
                                firstHalfOfSurrogatePair = cpFromHexDigits;
                            } else {
                                const auto secondHalfOSurrogatePair = cpFromHexDigits;
                                const auto encoding = encoder.Encode({
                                    ((firstHalfOfSurrogatePair - 0xD800) << 10)
                                    + (secondHalfOSurrogatePair - 0xDC00)
                                    + 0x10000
                                });
                                output += std::string(
                                    encoding.begin(),
                                    encoding.end()
                                );
                                firstHalfOfSurrogatePair = 0;
                            }
                        } else if (firstHalfOfSurrogatePair == 0) {
                            const auto encoding = encoder.Encode({cpFromHexDigits});
                            output += std::string(
                                encoding.begin(),
                                encoding.end()
                            );
                        } else {
                            return false;
                        }
                    }
                } break;
            }
        }
        return (
            (state != 1)
            && (
                (state < 2)
                || (state > 5)
            )
        );
    }

}

namespace Json {

    /**
     * This contains the private properties of a Json instance.
     */
    struct Json::Impl {
        // Properties

        /**
         * This indicates the type of the value represented
         * by the JSON object.
         */
        Type type = Type::Invalid;

        /**
         * This holds the actual value represented by the JSON
         * object.  Use the member that matches the type.
         */
        union {
            bool booleanValue;
            std::string* stringValue;
            std::vector< std::shared_ptr< Json > >* arrayValue;
            std::map< std::string, std::shared_ptr< Json > >* objectValue;
            int integerValue;
            double floatingPointValue;
        };

        /**
         * This is a cache of the encoding of the value.
         */
        std::string encoding;

        // Lifecycle management

        ~Impl() {
            switch (type) {
                case Type::String: {
                    delete stringValue;
                } break;

                case Type::Array: {
                    delete arrayValue;
                } break;

                case Type::Object: {
                    delete objectValue;
                } break;

                default: break;
            }
        }
        Impl(const Impl&) = delete;
        Impl(Impl&&) = delete;
        Impl& operator=(const Impl&) = delete;
        Impl& operator=(Impl&&) = delete;

        // Methods

        /**
         * This is the default constructor.
         */
        Impl() = default;

        /**
         * This method builds the JSON value up as a copy
         * of another JSON value.
         *
         * @param[in] other
         *     This is the other JSON value to copy.
         */
        void CopyFrom(const std::unique_ptr< Impl >& other) {
            type = other->type;
            switch (type) {
                case Type::Boolean: {
                    booleanValue = other->booleanValue;
                } break;

                case Type::Integer: {
                    integerValue = other->integerValue;
                } break;

                case Type::FloatingPoint: {
                    floatingPointValue = other->floatingPointValue;
                } break;

                case Type::String: {
                    stringValue = new std::string(*other->stringValue);
                } break;

                case Type::Array: {
                    arrayValue = new std::vector< std::shared_ptr< Json > >;
                    arrayValue->reserve(other->arrayValue->size());
                    for (const auto& otherElement: *other->arrayValue) {
                        const auto copy = std::make_shared< Json >(*otherElement);
                        arrayValue->push_back(copy);
                    }
                } break;

                case Type::Object: {
                    objectValue = new std::map< std::string, std::shared_ptr< Json > >;
                    for (const auto& otherElement: *other->objectValue) {
                        const auto copy = std::make_shared< Json >(*otherElement.second);
                        (*objectValue)[otherElement.first] = copy;
                    }
                } break;

                default: break;
            }
        }

        /**
         * This function parses the given string as an integer JSON value.
         *
         * @param[in] s
         *     This is the string to parse.
         */
        void ParseAsInteger(const std::string& s) {
            size_t index = 0;
            size_t state = 0;
            bool negative = false;
            int value = 0;
            while (index < s.length()) {
                switch (state) {
                    case 0: { // [ minus ]
                        if (s[index] == '-') {
                            negative = true;
                            ++index;
                        }
                        state = 1;
                    } break;

                    case 1: { // zero / 1-9
                        if (s[index] == '0') {
                            state = 2;
                        } else if (
                            (s[index] >= '1')
                            && (s[index] <= '9')
                        ) {
                            state = 3;
                            value = (int)(s[index] - '0');
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 2: { // extra junk!
                        return;
                    } break;

                    case 3: { // *DIGIT
                        if (
                            (s[index] >= '0')
                            && (s[index] <= '9')
                        ) {
                            const int previousValue = value;
                            value *= 10;
                            value += (int)(s[index] - '0');
                            if (value / 10 != previousValue) {
                                return;
                            }
                            ++index;
                        } else {
                            return;
                        }
                    } break;
                }
            }
            if (state >= 2) {
                type = Type::Integer;
                integerValue = (value * (negative ? -1 : 1));
            }
        }

        /**
         * This function parses the given string as a floating-point
         * JSON value.
         *
         * @param[in] s
         *     This is the string to parse.
         */
        void ParseAsFloatingPoint(const std::string& s) {
            size_t index = 0;
            size_t state = 0;
            bool negativeMagnitude = false;
            bool negativeExponent = false;
            double magnitude = 0.0;
            double fraction = 0.0;
            double exponent = 0.0;
            size_t fractionDigits = 0;
            while (index < s.length()) {
                switch (state) {
                    case 0: { // [ minus ]
                        if (s[index] == '-') {
                            negativeMagnitude = true;
                            ++index;
                        }
                        state = 1;
                    } break;

                    case 1: { // zero / 1-9
                        if (s[index] == '0') {
                            state = 2;
                        } else if (
                            (s[index] >= '1')
                            && (s[index] <= '9')
                        ) {
                            state = 3;
                            magnitude = (double)(s[index] - '0');
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 2: { // extra junk!
                        return;
                    } break;

                    case 3: { // *DIGIT / . / e / E
                        if (
                            (s[index] >= '0')
                            && (s[index] <= '9')
                        ) {
                            magnitude *= 10.0;
                            magnitude += (double)(s[index] - '0');
                        } else if (s[index] == '.') {
                            state = 4;
                        } else if (
                            (s[index] == 'e')
                            || (s[index] == 'E')
                        ) {
                            state = 6;
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 4: { // frac: DIGIT
                        if (
                            (s[index] >= '0')
                            && (s[index] <= '9')
                        ) {
                            ++fractionDigits;
                            fraction += (double)(s[index] - '0') / pow(10.0, (double)fractionDigits);
                        } else {
                            return;
                        }
                        state = 5;
                        ++index;
                    } break;

                    case 5: { // frac: *DIGIT / e / E
                        if (
                            (s[index] >= '0')
                            && (s[index] <= '9')
                        ) {
                            ++fractionDigits;
                            fraction += (double)(s[index] - '0') / pow(10.0, (double)fractionDigits);
                        } else if (
                            (s[index] == 'e')
                            || (s[index] == 'E')
                        ) {
                            state = 6;
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 6: { // exp: [minus/plus] / DIGIT
                        if (s[index] == '-') {
                            negativeExponent = true;
                            ++index;
                        } else if (s[index] == '+') {
                            ++index;
                        } else {
                        }
                        state = 7;
                    } break;

                    case 7: { // exp: DIGIT
                        state = 8;
                    } break;

                    case 8: { // exp: *DIGIT
                        if (
                            (s[index] >= '0')
                            && (s[index] <= '9')
                        ) {
                            exponent *= 10.0;
                            exponent += (double)(s[index] - '0');
                        } else {
                            return;
                        }
                        ++index;
                    } break;
                }
            }
            if (
                (state >= 2)
                && (state != 4)
                && (state != 6)
                && (state != 7)
            ) {
                type = Type::FloatingPoint;
                floatingPointValue = (
                    (
                        magnitude
                        + fraction
                    )
                    * pow(10.0, exponent * (negativeExponent ? -1.0 : 1.0))
                    * (negativeMagnitude ? -1.0 : 1.0)
                );
            }
        }

        /**
         * This function extracts the encoding of the next JSON value
         * in the given string, updating the given offset to indicate
         * where the end of the value encoding was found, or whether
         * the encoding was invalid.
         *
         * @param[in] s
         *     This is the string to parse.
         *
         * @param[in,out] offset
         *     On input, this is the position of the first character
         *     of the encoded value in the string.
         *
         *     On ouput, this is the position of the first character
         *     past the end of the encoded value in the string, or
         *     std::string::npos if the encoded value was invalid.
         *
         * @param[in] delimiter
         *     This is the character that marks the end of the value
         *     if it's encountered.
         *
         * @return
         *     The encoding of the next JSON value in the given string
         *     is returned.
         */
        std::string ParseValue(
            const std::string& s,
            size_t& offset,
            char delimiter
        ) {
            Utf8::Utf8 decoder, encoder;
            std::stack< char > expectedDelimiters;
            std::vector< Utf8::UnicodeCodePoint > encodedValueCodePoints;
            const auto encodingCodePoints = decoder.Decode(s.substr(offset));
            if (encodingCodePoints.empty()) {
                offset = std::string::npos;
                return "";
            }
            bool insideString = false;
            for (const auto cp: encodingCodePoints) {
                encodedValueCodePoints.push_back(cp);
                if (
                    !expectedDelimiters.empty()
                    && (cp == expectedDelimiters.top())
                ) {
                    insideString = false;
                    expectedDelimiters.pop();
                    continue;
                }
                if (!insideString) {
                    if (cp == (Utf8::UnicodeCodePoint)'\"') {
                        insideString = true;
                        expectedDelimiters.push('\"');
                    } else if (cp == (Utf8::UnicodeCodePoint)'[') {
                        expectedDelimiters.push(']');
                    } else if (
                        (cp == (Utf8::UnicodeCodePoint)delimiter)
                        && expectedDelimiters.empty()
                    ) {
                        break;
                    }
                }
            }
            if (expectedDelimiters.empty()) {
                auto encodedValue = encoder.Encode(encodedValueCodePoints);
                offset += encodedValue.size();
                if (encodedValue.back() == (Utf8::UnicodeCodePoint)delimiter) {
                    encodedValue.pop_back();
                }
                return std::string(
                    encodedValue.begin(),
                    encodedValue.end()
                );
            } else {
                offset = std::string::npos;
                return "";
            }
        }

        /**
         * This function parses the given string as an array
         * JSON value.
         *
         * @param[in] s
         *     This is the string to parse.
         */
        void ParseAsArray(const std::string& s) {
            std::vector< std::shared_ptr< Json > > newArrayValue;
            size_t offset = 0;
            while (offset < s.length()) {
                const auto encodedValue = ParseValue(s, offset, ',');
                if (offset == std::string::npos) {
                    return;
                }
                const auto value = std::make_shared< Json >(FromEncoding(encodedValue));
                newArrayValue.push_back(value);
            }
            type = Type::Array;
            arrayValue = new decltype(newArrayValue)(newArrayValue);
        }

        /**
         * This function parses the given string as an object
         * JSON value.
         *
         * @param[in] s
         *     This is the string to parse.
         */
        void ParseAsObject(const std::string& s) {
            std::map< std::string, std::shared_ptr< Json > > newObjectValue;
            size_t offset = 0;
            while (offset < s.length()) {
                const auto encodedKey = ParseValue(s, offset, ':');
                if (offset == std::string::npos) {
                    return;
                }
                const auto key = std::make_shared< Json >(FromEncoding(encodedKey));
                if (key->GetType() != Type::String) {
                    return;
                }
                const auto encodedValue = ParseValue(s, offset, ',');
                if (offset == std::string::npos) {
                    return;
                }
                const auto value = std::make_shared< Json >(FromEncoding(encodedValue));
                newObjectValue[(std::string)*key] = value;
            }
            type = Type::Object;
            objectValue = new decltype(newObjectValue)(newObjectValue);
        }
    };

    Json::~Json() = default;
    Json::Json(const Json& other)
        : impl_(new Impl)
    {
        impl_->CopyFrom(other.impl_);
    }
    Json::Json(Json&&) = default;
    Json& Json::operator=(Json&&) = default;
    Json& Json::operator=(const Json& other) {
        if (this != &other) {
            impl_.reset(new Impl());
            impl_->CopyFrom(other.impl_);
        }
        return *this;
    }

    Json::Json(Type type)
        : impl_(new Impl)
    {
        impl_->type = type;
        switch (type) {
            case Type::String: {
                impl_->stringValue = new std::string();
            } break;

            case Type::Array: {
                impl_->arrayValue = new std::vector< std::shared_ptr< Json > >;
            } break;

            case Type::Object: {
                impl_->objectValue = new std::map< std::string, std::shared_ptr< Json > >;
            } break;

            default: break;
        }
    }

    Json::Json(nullptr_t)
        : impl_(new Impl)
    {
        impl_->type = Type::Null;
    }

    Json::Json(bool value)
        : impl_(new Impl)
    {
        impl_->type = Type::Boolean;
        impl_->booleanValue = value;
    }

    Json::Json(int value)
        : impl_(new Impl)
    {
        impl_->type = Type::Integer;
        impl_->integerValue = value;
    }

    Json::Json(double value)
        : impl_(new Impl)
    {
        impl_->type = Type::FloatingPoint;
        impl_->floatingPointValue = value;
    }

    Json::Json(const char* value)
        : impl_(new Impl)
    {
        impl_->type = Type::String;
        impl_->stringValue = new std::string(value);
    }

    Json::Json(const std::string& value)
        : impl_(new Impl)
    {
        impl_->type = Type::String;
        impl_->stringValue = new std::string(value);
    }

    bool Json::operator==(const Json& other) const {
        if (impl_->type != other.impl_->type) {
            return false;
        } else switch (impl_->type) {
            case Type::Invalid: return true;

            case Type::Null: return true;

            case Type::Boolean: return impl_->booleanValue == other.impl_->booleanValue;

            case Type::String: return *impl_->stringValue == *other.impl_->stringValue;

            case Type::Integer: return impl_->integerValue == other.impl_->integerValue;

            case Type::FloatingPoint: return impl_->floatingPointValue == other.impl_->floatingPointValue;

            case Type::Array: {
                if (impl_->arrayValue->size() != other.impl_->arrayValue->size()) {
                    return false;
                }
                for (size_t i = 0; i < impl_->arrayValue->size(); ++i) {
                    if (*(*impl_->arrayValue)[i] != *(*other.impl_->arrayValue)[i]) {
                        return false;
                    }
                }
            } return true;

            case Type::Object: {
                std::set< std::string > keys;
                for (const auto& entry: *impl_->objectValue) {
                    (void)keys.insert(entry.first);
                }
                for (const auto& entry: *other.impl_->objectValue) {
                    const auto otherEntry = keys.find(entry.first);
                    if (otherEntry == keys.end()) {
                        return false;
                    }
                    (void)keys.erase(entry.first);
                }
                if (!keys.empty()) {
                    return false;
                }
                for (
                    auto it = impl_->objectValue->begin();
                    it != impl_->objectValue->end();
                    ++it
                ) {
                    if (*it->second != *(*other.impl_->objectValue)[it->first]) {
                        return false;
                    }
                }
            } return true;

            default: return true;
        }
    }

    bool Json::operator!=(const Json& other) const {
        return !(*this == other);
    }

    Json::operator bool() const {
        if (impl_->type == Type::Boolean) {
            return impl_->booleanValue;
        } else {
            return false;
        }
    }

    Json::operator std::string() const {
        if (impl_->type == Type::String) {
            return *impl_->stringValue;
        } else {
            return "";
        }
    }

    Json::operator int() const {
        if (impl_->type == Type::Integer) {
            return impl_->integerValue;
        } else if (impl_->type == Type::FloatingPoint) {
            return (int)impl_->floatingPointValue;
        } else {
            return 0;
        }
    }

    Json::operator double() const {
        if (impl_->type == Type::Integer) {
            return (double)impl_->integerValue;
        } else if (impl_->type == Type::FloatingPoint) {
            return impl_->floatingPointValue;
        } else {
            return 0.0;
        }
    }

    auto Json::GetType() const -> Type {
        return impl_->type;
    }

    size_t Json::GetSize() const {
        if (impl_->type == Type::Array) {
            return impl_->arrayValue->size();
        } else if (impl_->type == Type::Object) {
            return impl_->objectValue->size();
        } else {
            return 0;
        }
    }

    bool Json::Has(const std::string& key) const {
        if (impl_->type == Type::Object) {
            return (impl_->objectValue->find(key) != impl_->objectValue->end());
        } else {
            return false;
        }
    }

    std::shared_ptr< Json > Json::operator[](size_t index) const {
        if (impl_->type == Type::Array) {
            if (index >= impl_->arrayValue->size()) {
                return nullptr;
            }
            return (*impl_->arrayValue)[index];
        } else {
            return nullptr;;
        }
    }

    std::shared_ptr< Json > Json::operator[](int index) const {
        return (*this)[(size_t)index];
    }

    std::shared_ptr< Json > Json::operator[](const std::string& key) const {
        if (impl_->type == Type::Object) {
            const auto entry = impl_->objectValue->find(key);
            if (entry == impl_->objectValue->end()) {
                return nullptr;
            }
            return entry->second;
        } else {
            return nullptr;
        }
    }

    std::shared_ptr< Json > Json::operator[](const char* key) const {
        return (*this)[std::string(key)];
    }

    void Json::Add(const Json& value) {
        if (impl_->type != Type::Array) {
            return;
        }
        Insert(value, impl_->arrayValue->size());
        impl_->encoding.clear();
    }

    void Json::Insert(const Json& value, size_t index) {
        if (impl_->type != Type::Array) {
            return;
        }
        (void)impl_->arrayValue->insert(
            impl_->arrayValue->begin() + std::min(
                index,
                impl_->arrayValue->size()
            ),
            std::make_shared< Json >(value)
        );
        impl_->encoding.clear();
    }

    void Json::Set(
        const std::string& key,
        const Json& value
    ) {
        if (impl_->type != Type::Object) {
            return;
        }
        (*impl_->objectValue)[key] = std::make_shared< Json >(value);
        impl_->encoding.clear();
    }

    void Json::Remove(size_t index) {
        if (impl_->type != Type::Array) {
            return;
        }
        if (index < impl_->arrayValue->size()) {
            impl_->arrayValue->erase(
                impl_->arrayValue->begin() + index
            );
            impl_->encoding.clear();
        }
    }

    void Json::Remove(const std::string& key) {
        if (impl_->type != Type::Object) {
            return;
        }
        (void)impl_->objectValue->erase(key);
        impl_->encoding.clear();
    }

    std::string Json::ToEncoding(const EncodingOptions& options) const {
        if (impl_->type == Type::Invalid) {
            return SystemAbstractions::sprintf(
                "(Invalid JSON: %s)",
                impl_->encoding.c_str()
            );
        }
        if (options.reencode) {
            impl_->encoding.clear();
        }
        if (impl_->encoding.empty()) {
            switch (impl_->type) {
                case Type::Null: {
                    impl_->encoding = "null";
                } break;

                case Type::Boolean: {
                    impl_->encoding = impl_->booleanValue ? "true" : "false";
                } break;

                case Type::String: {
                    impl_->encoding = (
                        "\""
                        + Escape(*impl_->stringValue, options)
                        + "\""
                    );
                } break;

                case Type::Integer: {
                    impl_->encoding = SystemAbstractions::sprintf("%i", impl_->integerValue);
                } break;

                case Type::FloatingPoint: {
                    impl_->encoding = SystemAbstractions::sprintf("%lg", impl_->floatingPointValue);
                } break;

                case Type::Array: {
                    impl_->encoding = '[';
                    bool isFirst = true;
                    for (const auto value: *impl_->arrayValue) {
                        if (isFirst) {
                            isFirst = false;
                        } else {
                            impl_->encoding += ',';
                        }
                        impl_->encoding += value->ToEncoding(options);
                    }
                    impl_->encoding += ']';
                } break;

                case Type::Object: {
                    impl_->encoding = '{';
                    bool isFirst = true;
                    for (const auto& entry: *impl_->objectValue) {
                        if (isFirst) {
                            isFirst = false;
                        } else {
                            impl_->encoding += ',';
                        }
                        const Json keyAsJson(entry.first);
                        impl_->encoding += keyAsJson.ToEncoding(options);
                        impl_->encoding += ':';
                        impl_->encoding += entry.second->ToEncoding(options);
                    }
                    impl_->encoding += '}';
                } break;

                default: {
                    impl_->encoding = "???";
                } break;
            }
        }
        return impl_->encoding;
    }

    Json Json::FromEncoding(const std::string& encodingBeforeTrim) {
        Json json;
        const auto firstNonWhitespaceCharacter = encodingBeforeTrim.find_first_not_of(WHITESPACE_CHARACTERS);
        if (firstNonWhitespaceCharacter == std::string::npos) {
            return json;
        }
        const auto firstLastNonWhitespaceCharacter = encodingBeforeTrim.find_last_not_of(WHITESPACE_CHARACTERS);
        const auto encoding = encodingBeforeTrim.substr(
            firstNonWhitespaceCharacter,
            firstLastNonWhitespaceCharacter - firstNonWhitespaceCharacter + 1
        );
        json.impl_->encoding = encoding;
        if (encoding.empty()) {
        } else if (
            (encoding[0] == '{')
            && (encoding[encoding.length() - 1] == '}')
        ) {
            json.impl_->ParseAsObject(encoding.substr(1, encoding.length() - 2));
        } else if (
            (encoding[0] == '[')
            && (encoding[encoding.length() - 1] == ']')
        ) {
            json.impl_->ParseAsArray(encoding.substr(1, encoding.length() - 2));
        } else if (
            (encoding[0] == '"')
            && (encoding[encoding.length() - 1] == '"')
        ) {
            std::string output;
            if (
                Unescape(
                    encoding.substr(1, encoding.length() - 2),
                    output
                )
            ) {
                json.impl_->type = Type::String;
                json.impl_->stringValue = new std::string(output);
            }
        } else if (encoding == "null") {
            json.impl_->type = Type::Null;
        } else if (encoding == "true") {
            json.impl_->type = Type::Boolean;
            json.impl_->booleanValue = true;
        } else if (encoding == "false") {
            json.impl_->type = Type::Boolean;
            json.impl_->booleanValue = false;
        } else {
            if (encoding.find_first_of("+.eE") != std::string::npos) {
                json.impl_->ParseAsFloatingPoint(encoding);
            } else {
                json.impl_->ParseAsInteger(encoding);
            }
        }
        return json;
    }

    void PrintTo(
        Json::Type type,
        std::ostream* os
    ) {
        switch (type) {
            case Json::Type::Invalid: {
                *os << "Invalid";
            } break;

            case Json::Type::Boolean: {
                *os << "Boolean";
            } break;

            case Json::Type::String: {
                *os << "String";
            } break;

            case Json::Type::Integer: {
                *os << "Integer";
            } break;

            case Json::Type::FloatingPoint: {
                *os << "FloatingPoint";
            } break;

            case Json::Type::Array: {
                *os << "Array";
            } break;

            case Json::Type::Object: {
                *os << "Object";
            } break;

            default: {
                *os << "???";
            } break;
        }
    }

    void PrintTo(
        const Json& json,
        std::ostream* os
    ) {
        *os << json.ToEncoding();
    }

}
