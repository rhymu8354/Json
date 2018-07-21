/**
 * @file Json.cpp
 *
 * This module contains the implementation of the Json::Json class.
 *
 * © 2018 by Richard Walters
 */

#include <Json/Json.hpp>
#include <map>
#include <math.h>
#include <string>
#include <SystemAbstractions/StringExtensions.hpp>
#include <Utf8/Utf8.hpp>

namespace {

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
     * This function parses the given string as an integer JSON value.
     *
     * @param[in] s
     *     This is the string to parse.
     *
     * @return
     *     The parsed JSON value is returned.
     */
    Json::Json ParseInteger(const std::string& s) {
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
                        return Json::Json();
                    }
                    ++index;
                } break;

                case 2: { // extra junk!
                    return Json::Json();
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
                            return Json::Json();
                        }
                        ++index;
                    } else {
                        return Json::Json();
                    }
                } break;
            }
        }
        if (state < 2) {
            return Json::Json();
        } else {
            return Json::Json(value * (negative ? -1 : 1));
        }
    }

    /**
     * This function parses the given string as a floating-point JSON value.
     *
     * @param[in] s
     *     This is the string to parse.
     *
     * @return
     *     The parsed JSON value is returned.
     */
    Json::Json ParseFloatingPoint(const std::string& s) {
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
                        return Json::Json();
                    }
                    ++index;
                } break;

                case 2: { // extra junk!
                    return Json::Json();
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
                        return Json::Json();
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
                        return Json::Json();
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
                        return Json::Json();
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
                        return Json::Json();
                    }
                    ++index;
                } break;
            }
        }
        if (
            (state < 2)
            || (state == 4)
            || (state == 6)
            || (state == 7)
        ) {
            return Json::Json();
        } else {
            return Json::Json(
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
                output += "\\u";
                output += CodePointToFourHexDigits(cp);
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
     * @return
     *     The unescaped string is returned.
     */
    std::string Unescape(const std::string& s) {
        Utf8::Utf8 decoder, encoder;
        std::string output;
        size_t state = 0;
        Utf8::UnicodeCodePoint cpFromHexDigits = 0;
        std::vector< Utf8::UnicodeCodePoint > hexDigitsOriginal;
        for (const auto cp: decoder.Decode(s)) {
            switch (state) {
                case 0: { // initial state
                    if (cp == 0x5C) { // '\\'
                        state = 1;
                    } else {
                        const auto encoding = encoder.Encode({cp});
                        output += std::string(
                            encoding.begin(),
                            encoding.end()
                        );
                    }
                } break;

                case 1: { // escape character
                    if (cp == 0x75) { // 'u'
                        state = 2;
                        cpFromHexDigits = 0;
                        hexDigitsOriginal = { 0x5C, 0x75 };
                    } else {
                        Utf8::UnicodeCodePoint alternative = cp;
                        const auto entry = SPECIAL_ESCAPE_DECODINGS.find(cp);
                        if (entry == SPECIAL_ESCAPE_DECODINGS.end()) {
                            const auto encoding = encoder.Encode({ 0x5C, cp });
                            output += std::string(
                                encoding.begin(),
                                encoding.end()
                            );
                        } else {
                            const auto encoding = encoder.Encode({entry->second});
                            output += std::string(
                                encoding.begin(),
                                encoding.end()
                            );
                        }
                        state = 0;
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
                        state = 0;
                        const auto encoding = encoder.Encode(hexDigitsOriginal);
                        output += std::string(
                            encoding.begin(),
                            encoding.end()
                        );
                        break;
                    }
                    if (++state == 6) {
                        state = 0;
                        const auto encoding = encoder.Encode({cpFromHexDigits});
                        output += std::string(
                            encoding.begin(),
                            encoding.end()
                        );
                    }
                } break;
            }
        }
        switch (state) {
            case 1: { // escape character
                const auto encoding = encoder.Encode({0x5C});
                output += std::string(
                    encoding.begin(),
                    encoding.end()
                );
            } break;

            case 2:   // first hexdigit of escape uXXXX
            case 3:   // second hexdigit of escape uXXXX
            case 4:   // third hexdigit of escape uXXXX
            case 5: { // fourth hexdigit of escape uXXXX
                const auto encoding = encoder.Encode(hexDigitsOriginal);
                output += std::string(
                    encoding.begin(),
                    encoding.end()
                );
            } break;
        }
        return output;
    }

}

namespace Json {

    /**
     * This contains the private properties of a Json instance.
     */
    struct Json::Impl {
        // Properties

        /**
         * These are the different kinds of values that a JSON object can be.
         */
        enum class Type {
            Invalid,
            Null,
            Boolean,
            String,
            Integer,
            FloatingPoint,
        };

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
            int integerValue;
            double floatingPointValue;
        };

        // Lifecycle management

        ~Impl() {
            switch (type) {
                case Impl::Type::String: {
                    delete stringValue;
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
    };

    Json::~Json() = default;
    Json::Json(Json&&) = default;
    Json& Json::operator=(Json&&) = default;

    Json::Json()
        : impl_(new Impl)
    {
    }

    Json::Json(nullptr_t)
        : impl_(new Impl)
    {
        impl_->type = Impl::Type::Null;
    }

    Json::Json(bool value)
        : impl_(new Impl)
    {
        impl_->type = Impl::Type::Boolean;
        impl_->booleanValue = value;
    }

    Json::Json(int value)
        : impl_(new Impl)
    {
        impl_->type = Impl::Type::Integer;
        impl_->integerValue = value;
    }

    Json::Json(double value)
        : impl_(new Impl)
    {
        impl_->type = Impl::Type::FloatingPoint;
        impl_->floatingPointValue = value;
    }

    Json::Json(const char* value)
        : impl_(new Impl)
    {
        impl_->type = Impl::Type::String;
        impl_->stringValue = new std::string(value);
    }

    Json::Json(const std::string& value)
        : impl_(new Impl)
    {
        impl_->type = Impl::Type::String;
        impl_->stringValue = new std::string(value);
    }

    bool Json::operator==(const Json& other) const {
        if (impl_->type != other.impl_->type) {
            return false;
        } else switch(impl_->type) {
            case Impl::Type::Null: return true;
            case Impl::Type::Boolean: return impl_->booleanValue == other.impl_->booleanValue;
            case Impl::Type::String: return *impl_->stringValue == *other.impl_->stringValue;
            case Impl::Type::Integer: return impl_->integerValue == other.impl_->integerValue;
            case Impl::Type::FloatingPoint: return impl_->floatingPointValue == other.impl_->floatingPointValue;
            default: return true;
        }
    }

    Json::operator bool() const {
        if (impl_->type == Impl::Type::Boolean) {
            return impl_->booleanValue;
        } else {
            return false;
        }
    }

    Json::operator std::string() const {
        if (impl_->type == Impl::Type::String) {
            return *impl_->stringValue;
        } else {
            return "";
        }
    }

    Json::operator int() const {
        if (impl_->type == Impl::Type::Integer) {
            return impl_->integerValue;
        } else if (impl_->type == Impl::Type::FloatingPoint) {
            return (int)impl_->floatingPointValue;
        } else {
            return 0;
        }
    }

    Json::operator double() const {
        if (impl_->type == Impl::Type::Integer) {
            return (double)impl_->integerValue;
        } else if (impl_->type == Impl::Type::FloatingPoint) {
            return impl_->floatingPointValue;
        } else {
            return 0.0;
        }
    }

    std::string Json::ToEncoding(const EncodingOptions& options) const {
        switch (impl_->type) {
            case Impl::Type::Null: return "null";
            case Impl::Type::Boolean: return impl_->booleanValue ? "true" : "false";
            case Impl::Type::String: return (
                "\""
                + Escape(*impl_->stringValue, options)
                + "\""
            );
            case Impl::Type::Integer: return SystemAbstractions::sprintf("%i", impl_->integerValue);
            case Impl::Type::FloatingPoint: return SystemAbstractions::sprintf("%lg", impl_->floatingPointValue);
            default: return "???";
        }
    }

    Json Json::FromEncoding(const std::string& encoding) {
        if (encoding.empty()) {
            return Json();
        } else if (encoding[0] == '{') {
            // TODO: parse as object
            return Json();
        } else if (
            !encoding.empty()
            && (encoding[0] == '[')
        ) {
            // TODO: parse as array
            return Json();
        } else if (
            (encoding[0] == '"')
            && (encoding[encoding.length() - 1] == '"')
        ) {
            return Unescape(encoding.substr(1, encoding.length() - 2));
        } else if (encoding == "null") {
            return nullptr;
        } else if (encoding == "true") {
            return true;
        } else if (encoding == "false") {
            return false;
        } else {
            if (encoding.find_first_of("+.eE") != std::string::npos) {
                return ParseFloatingPoint(encoding);
            } else {
                return ParseInteger(encoding);
            }
        }
    }

}
