/**
 * @file Json.cpp
 *
 * This module contains the implementation of the Json::Json class.
 *
 * © 2018 by Richard Walters
 */

#include <algorithm>
#include <Json/Json.hpp>
#include <limits>
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
     * This is returned from indexers when indexed values are
     * not found.
     */
    const Json::Json null = nullptr;

    /**
     * These are the character that are considered "whitespace"
     * by the JSON standard (RFC 7159).
     */
    std::set< Utf8::UnicodeCodePoint > WHITESPACE_CHARACTERS{
        0x20, // ' '
        0x09, // '\t'
        0x0D, // '\r'
        0x0A, // '\n'
    };

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
     * This method finds the offset of the first code point in the
     * given vector of code points that is not in the given search set.
     *
     * @param[in] codePoints
     *     This is the vector of code points to search.
     *
     * @param[in] searchSet
     *     This is the set of code points to skip.
     *
     * @param[in] forwardDirection
     *     This indicates whether or not to search forward from the front,
     *     rather than backwards from the back.
     *
     * @return
     *     The offset of the first code point that isn't in the given
     *     search set is returned.
     *
     * @retval codePoints.size()
     *     This is returned if all of the code points are in the search set.
     */
    size_t FindFirstNotOf(
        const std::vector< Utf8::UnicodeCodePoint >& codePoints,
        const std::set< Utf8::UnicodeCodePoint > searchSet,
        bool forwardDirection
    ) {
        size_t offset = 0;
        while (offset < codePoints.size()) {
            const auto entry = searchSet.find(
                forwardDirection
                ? codePoints[offset]
                : codePoints[codePoints.size() - offset - 1]
            );
            if (entry == searchSet.end()) {
                break;
            }
            ++offset;
        }
        if (offset < codePoints.size()) {
            return forwardDirection ? offset : codePoints.size() - offset - 1;
        } else {
            return offset;
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
     * This function returns the JSON encoding of the given string.
     *
     * @param[in] s
     *     This is the string which needs to be escaped.
     *
     * @param[in] options
     *     This is used to configure various options having to do with
     *     encoding a Json object into its string format.
     *
     * @return
     *     The encoded string is returned.
     */
    std::string EncodeString(
        const std::string& s,
        const Json::EncodingOptions& options
    ) {
        Utf8::Utf8 utf8;
        std::string output;
        for (const auto cp: utf8.Decode(s)) {
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
                const auto encoding = utf8.Encode({cp});
                output += std::string(
                    encoding.begin(),
                    encoding.end()
                );
            }
        }
        return output;
    }

    /**
     * This function return the JSON decoding of the given string.
     *
     * @param[in] s
     *     This is the JSON-encoded string which needs to be decoded.
     *
     * @param[out] output
     *     This is where to put the unescaped string.
     *
     * @return
     *     An indication of whether or not the input string was a valid
     *     JSON encoding is returned.
     */
    bool DecodeString(
        const std::string& s,
        std::string& output
    ) {
        Utf8::Utf8 utf8;
        size_t state = 0;
        Utf8::UnicodeCodePoint cpFromHexDigits = 0;
        Utf8::UnicodeCodePoint firstHalfOfSurrogatePair = 0;
        std::vector< Utf8::UnicodeCodePoint > hexDigitsOriginal;
        for (const auto cp: utf8.Decode(s)) {
            switch (state) {
                case 0: { // initial state
                    if (cp == 0x5C) { // '\\'
                        state = 1;
                    } else if (firstHalfOfSurrogatePair == 0) {
                        const auto encoding = utf8.Encode({cp});
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
                        const auto encoding = utf8.Encode({entry->second});
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
                                const auto encoding = utf8.Encode({
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
                            const auto encoding = utf8.Encode({cpFromHexDigits});
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

    /**
     * This function performs a deep comparison of two arrays
     * of JSON values.
     *
     * @param[in] lhs
     *     This is the first array of JSON values to compare.
     *
     * @param[in] rhs
     *     This is the second array of JSON values to compare.
     *
     * @return
     *     An indication of whether or not the two given
     *     arrays of JSON values are considered equalivalent
     *     is returned.
     */
    bool CompareJsonArrays(
        const std::vector< Json::Json >&lhs,
        const std::vector< Json::Json >&rhs
    ) {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        for (size_t i = 0; i < lhs.size(); ++i) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }
        return true;
    }

    /**
     * This function performs a deep comparison of two arrays
     * of JSON values.
     *
     * @param[in] lhs
     *     This is the first array of JSON values to compare.
     *
     * @param[in] rhs
     *     This is the second array of JSON values to compare.
     *
     * @return
     *     An indication of whether or not the two given
     *     arrays of JSON values are considered equalivalent
     *     is returned.
     */
    bool CompareJsonObjects(
        const std::map< std::string, Json::Json >&lhs,
        const std::map< std::string, Json::Json >&rhs
    ) {
        std::set< std::string > keys;
        for (const auto& entry: lhs) {
            (void)keys.insert(entry.first);
        }
        for (const auto& entry: rhs) {
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
            auto it = lhs.begin();
            it != lhs.end();
            ++it
        ) {
            const auto otherEntry = rhs.find(it->first);
            if (it->second != otherEntry->second) {
                return false;
            }
        }
        return true;
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
            std::vector< Json >* arrayValue;
            std::map< std::string, Json >* objectValue;
            int integerValue;
            double floatingPointValue;
        };

        /**
         * This is a cache of the encoding of the value.
         */
        std::string encoding;

        // Lifecycle management

        ~Impl() noexcept {
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
        Impl(Impl&&) noexcept = delete;
        Impl& operator=(const Impl&) = delete;
        Impl& operator=(Impl&&) noexcept = delete;

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
                    arrayValue = new std::vector< Json >;
                    arrayValue->reserve(other->arrayValue->size());
                    for (const auto& otherElement: *other->arrayValue) {
                        arrayValue->emplace_back(otherElement);
                    }
                } break;

                case Type::Object: {
                    objectValue = new std::map< std::string, Json >;
                    for (const auto& otherElement: *other->objectValue) {
                        objectValue->insert({ otherElement.first, otherElement.second });
                    }
                } break;

                default: break;
            }
        }

        /**
         * This function decodes the given code point sequence
         * as an integer JSON value.
         *
         * @param[in] codePoints
         *     This is the code point sequence to decode.
         */
        void DecodeAsInteger(const std::vector< Utf8::UnicodeCodePoint >& codePoints) {
            Utf8::Utf8 encoder;
            const auto s = encoder.Encode(codePoints);
            intmax_t value;
            if (
                SystemAbstractions::ToInteger(
                    std::string(s.begin(), s.end()),
                    value
                ) != SystemAbstractions::ToIntegerResult::Success
            ) {
                return;
            }
            if (
                (value < (decltype(value))std::numeric_limits< decltype(integerValue) >::lowest())
                || (value > (decltype(value))std::numeric_limits< decltype(integerValue) >::max())
            ) {
                return;
            }
            type = Type::Integer;
            integerValue = (decltype(integerValue))value;
        }

        /**
         * This function decodes the given code point sequence as
         * a floating-point JSON value.
         *
         * @param[in] codePoints
         *     This is the code point sequence to parse.
         */
        void DecodeAsFloatingPoint(const std::vector< Utf8::UnicodeCodePoint >& codePoints) {
            size_t index = 0;
            size_t state = 0;
            bool negativeMagnitude = false;
            bool negativeExponent = false;
            double magnitude = 0.0;
            double fraction = 0.0;
            double exponent = 0.0;
            size_t fractionDigits = 0;
            while (index < codePoints.size()) {
                switch (state) {
                    case 0: { // [ minus ]
                        if (codePoints[index] == (Utf8::UnicodeCodePoint)'-') {
                            negativeMagnitude = true;
                            ++index;
                        }
                        state = 1;
                    } break;

                    case 1: { // zero / 1-9
                        if (codePoints[index] == (Utf8::UnicodeCodePoint)'0') {
                            state = 2;
                        } else if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'1')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            state = 3;
                            magnitude = (double)(codePoints[index] - (Utf8::UnicodeCodePoint)'0');
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 2: { // . / e / E
                        if (codePoints[index] == (Utf8::UnicodeCodePoint)'.') {
                            state = 4;
                        } else if (
                            (codePoints[index] == (Utf8::UnicodeCodePoint)'e')
                            || (codePoints[index] == (Utf8::UnicodeCodePoint)'E')
                        ) {
                            state = 6;
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 3: { // *DIGIT / . / e / E
                        if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'0')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            const auto oldMagnitude = (intmax_t)magnitude;
                            magnitude *= 10.0;
                            magnitude += (double)(codePoints[index] - (Utf8::UnicodeCodePoint)'0');
                            if ((intmax_t)magnitude / 10 != oldMagnitude) {
                                return;
                            }
                        } else if (codePoints[index] == (Utf8::UnicodeCodePoint)'.') {
                            state = 4;
                        } else if (
                            (codePoints[index] == (Utf8::UnicodeCodePoint)'e')
                            || (codePoints[index] == (Utf8::UnicodeCodePoint)'E')
                        ) {
                            state = 6;
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 4: { // frac: DIGIT
                        if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'0')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            ++fractionDigits;
                            fraction += (
                                (double)(
                                    codePoints[index] - (Utf8::UnicodeCodePoint)'0'
                                )
                                / pow(10.0, (double)fractionDigits)
                            );
                        } else {
                            return;
                        }
                        state = 5;
                        ++index;
                    } break;

                    case 5: { // frac: *DIGIT / e / E
                        if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'0')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            ++fractionDigits;
                            fraction += (
                                (double)(
                                    codePoints[index] - (Utf8::UnicodeCodePoint)'0'
                                )
                                / pow(10.0, (double)fractionDigits)
                            );
                        } else if (
                            (codePoints[index] == (Utf8::UnicodeCodePoint)'e')
                            || (codePoints[index] == (Utf8::UnicodeCodePoint)'E')
                        ) {
                            state = 6;
                        } else {
                            return;
                        }
                        ++index;
                    } break;

                    case 6: { // exp: [minus/plus] / DIGIT
                        if (codePoints[index] == (Utf8::UnicodeCodePoint)'-') {
                            negativeExponent = true;
                            ++index;
                        } else if (codePoints[index] == (Utf8::UnicodeCodePoint)'+') {
                            ++index;
                        } else {
                        }
                        state = 7;
                    } break;

                    case 7: { // exp: DIGIT
                        if (
                            (codePoints[index] >= (Utf8::UnicodeCodePoint)'0')
                            && (codePoints[index] <= (Utf8::UnicodeCodePoint)'9')
                        ) {
                            const auto oldExponent = (intmax_t)exponent;
                            exponent *= 10.0;
                            exponent += (double)(codePoints[index] - (Utf8::UnicodeCodePoint)'0');
                            if ((intmax_t)exponent / 10 != oldExponent) {
                                return;
                            }
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
         * @param[in] codePoints
         *     These are the Unicode code points to parse.
         *
         * @param[in,out] offset
         *     On input, this is the position of the first character
         *     of the encoded value in the string.
         *
         *     On ouput, this is the position of the first character
         *     past the end of the encoded value in the string
         *
         * @param[in] delimiter
         *     This is the character that marks the end of the value
         *     if it's encountered.
         *
         * @return
         *     The encoding of the next JSON value in the given
         *     Unicode code points is returned.
         *
         * @return {}
         *     This is returned if the encoded value was invalid.
         */
        std::vector< Utf8::UnicodeCodePoint > ParseValue(
            const std::vector< Utf8::UnicodeCodePoint >& codePoints,
            size_t& offset,
            char delimiter
        ) {
            std::stack< char > expectedDelimiters;
            std::vector< Utf8::UnicodeCodePoint > encodedValueCodePoints;
            const std::vector< Utf8::UnicodeCodePoint > encodingCodePoints(
                codePoints.begin() + offset,
                codePoints.end()
            );
            if (encodingCodePoints.empty()) {
                return {};
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
                    } else if (cp == (Utf8::UnicodeCodePoint)'{') {
                        expectedDelimiters.push('}');
                    } else if (
                        (cp == (Utf8::UnicodeCodePoint)delimiter)
                        && expectedDelimiters.empty()
                    ) {
                        break;
                    }
                }
            }
            if (expectedDelimiters.empty()) {
                offset += encodedValueCodePoints.size();
                if (encodedValueCodePoints.back() == (Utf8::UnicodeCodePoint)delimiter) {
                    encodedValueCodePoints.pop_back();
                }
                return encodedValueCodePoints;
            } else {
                return {};
            }
        }

        /**
         * This function parses the given Unicode code points as an array
         * JSON value.
         *
         * @param[in] codePoints
         *     These are the Unicode code points to parse.
         */
        void ParseAsArray(const std::vector< Utf8::UnicodeCodePoint >& codePoints) {
            std::vector< Json > newArrayValue;
            size_t offset = 0;
            while (offset < codePoints.size()) {
                const auto encodedValue = ParseValue(codePoints, offset, ',');
                if (encodedValue.empty()) {
                    return;
                }
                newArrayValue.emplace_back(FromEncoding(encodedValue));
            }
            type = Type::Array;
            arrayValue = new decltype(newArrayValue)(newArrayValue);
        }

        /**
         * This function parses the given Unicode code points as an object
         * JSON value.
         *
         * @param[in] codePoints
         *     These are the Unicode code points to parse.
         */
        void ParseAsObject(const std::vector< Utf8::UnicodeCodePoint >& codePoints) {
            std::map< std::string, Json > newObjectValue;
            size_t offset = 0;
            while (offset < codePoints.size()) {
                const auto encodedKey = ParseValue(codePoints, offset, ':');
                if (encodedKey.empty()) {
                    return;
                }
                const auto key = FromEncoding(encodedKey);
                if (key.GetType() != Type::String) {
                    return;
                }
                const auto encodedValue = ParseValue(codePoints, offset, ',');
                if (encodedValue.empty()) {
                    return;
                }
                newObjectValue[(std::string)key] = FromEncoding(encodedValue);
            }
            type = Type::Object;
            objectValue = new decltype(newObjectValue)(newObjectValue);
        }
    };

    Json::~Json() noexcept = default;
    Json::Json(const Json& other)
        : impl_(new Impl)
    {
        impl_->CopyFrom(other.impl_);
    }
    Json::Json(Json&&) noexcept = default;
    Json& Json::operator=(Json&&) noexcept = default;
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
                impl_->arrayValue = new std::vector< Json >;
            } break;

            case Type::Object: {
                impl_->objectValue = new std::map< std::string, Json >;
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
            case Type::Array: return CompareJsonArrays(*impl_->arrayValue, *other.impl_->arrayValue);
            case Type::Object: return CompareJsonObjects(*impl_->objectValue, *other.impl_->objectValue);
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

    std::vector< std::string > Json::GetKeys() const {
        std::vector< std::string > keys;
        if (impl_->type == Type::Object) {
            keys.reserve(impl_->objectValue->size());
            for (const auto& entry: *impl_->objectValue) {
                keys.push_back(entry.first);
            }
        }
        return keys;
    }

    const Json& Json::operator[](size_t index) const {
        if (impl_->type == Type::Array) {
            if (index >= impl_->arrayValue->size()) {
                return null;
            }
            return (*impl_->arrayValue)[index];
        } else {
            return null;
        }
    }

    const Json& Json::operator[](int index) const {
        return (*this)[(size_t)index];
    }

    const Json& Json::operator[](const std::string& key) const {
        if (impl_->type == Type::Object) {
            const auto entry = impl_->objectValue->find(key);
            if (entry == impl_->objectValue->end()) {
                return null;
            }
            return entry->second;
        } else {
            return null;
        }
    }

    const Json& Json::operator[](const char* key) const {
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
            value
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
        (*impl_->objectValue)[key] = value;
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
                        + EncodeString(*impl_->stringValue, options)
                        + "\""
                    );
                } break;

                case Type::Integer: {
                    impl_->encoding = SystemAbstractions::sprintf("%i", impl_->integerValue);
                } break;

                case Type::FloatingPoint: {
                    impl_->encoding = SystemAbstractions::sprintf("%lg", impl_->floatingPointValue);
                    if (impl_->encoding.find_first_not_of("0123456789-") == std::string::npos) {
                        impl_->encoding += ".0";
                    }
                } break;

                case Type::Array: {
                    impl_->encoding = '[';
                    bool isFirst = true;
                    auto nestedOptions = options;
                    ++nestedOptions.numIndentationLevels;
                    std::string nestedIndentation(
                        (
                            nestedOptions.numIndentationLevels
                            * nestedOptions.spacesPerIndentationLevel
                        ),
                        ' '
                    );
                    std::string wrappedEncoding = "[\r\n";
                    for (const auto value: *impl_->arrayValue) {
                        if (isFirst) {
                            isFirst = false;
                        } else {
                            impl_->encoding += (nestedOptions.pretty ? ", " : ",");
                            wrappedEncoding += ",\r\n";
                        }
                        const auto encodedValue = value.ToEncoding(nestedOptions);
                        impl_->encoding += encodedValue;
                        wrappedEncoding += nestedIndentation;
                        wrappedEncoding += encodedValue;
                    }
                    impl_->encoding += ']';
                    std::string indentation(
                        (
                            options.numIndentationLevels
                            * options.spacesPerIndentationLevel
                        ),
                        ' '
                    );
                    wrappedEncoding += "\r\n";
                    wrappedEncoding += indentation;
                    wrappedEncoding += "]";
                    if (
                        options.pretty
                        && (indentation.length() + impl_->encoding.length() > options.wrapThreshold)
                    ) {
                        impl_->encoding = wrappedEncoding;
                    }
                } break;

                case Type::Object: {
                    impl_->encoding = '{';
                    bool isFirst = true;
                    auto nestedOptions = options;
                    ++nestedOptions.numIndentationLevels;
                    std::string nestedIndentation(
                        (
                            nestedOptions.numIndentationLevels
                            * nestedOptions.spacesPerIndentationLevel
                        ),
                        ' '
                    );
                    std::string wrappedEncoding = "{\r\n";
                    for (const auto& entry: *impl_->objectValue) {
                        if (isFirst) {
                            isFirst = false;
                        } else {
                            impl_->encoding += (nestedOptions.pretty ? ", " : ",");
                            wrappedEncoding += ",\r\n";
                        }
                        const Json keyAsJson(entry.first);
                        const auto encodedValue = (
                            keyAsJson.ToEncoding(nestedOptions)
                            + (nestedOptions.pretty ? ": " : ":")
                            + entry.second.ToEncoding(nestedOptions)
                        );
                        impl_->encoding += encodedValue;
                        wrappedEncoding += nestedIndentation;
                        wrappedEncoding += encodedValue;
                    }
                    impl_->encoding += '}';
                    std::string indentation(
                        (
                            options.numIndentationLevels
                            * options.spacesPerIndentationLevel
                        ),
                        ' '
                    );
                    wrappedEncoding += "\r\n";
                    wrappedEncoding += indentation;
                    wrappedEncoding += "}";
                    if (
                        options.pretty
                        && (indentation.length() + impl_->encoding.length() > options.wrapThreshold)
                    ) {
                        impl_->encoding = wrappedEncoding;
                    }
                } break;

                default: {
                    impl_->encoding = "???";
                } break;
            }
        }
        return impl_->encoding;
    }

    Json Json::FromEncoding(const std::vector< Utf8::UnicodeCodePoint >& encodingBeforeTrim) {
        Json json;
        const auto firstNonWhitespaceCharacter = FindFirstNotOf(
            encodingBeforeTrim,
            WHITESPACE_CHARACTERS,
            true
        );
        if (firstNonWhitespaceCharacter == encodingBeforeTrim.size()) {
            return json;
        }
        const auto lastNonWhitespaceCharacter = FindFirstNotOf(
            encodingBeforeTrim,
            WHITESPACE_CHARACTERS,
            false
        );
        const std::vector< Utf8::UnicodeCodePoint > encoding(
            encodingBeforeTrim.begin() + firstNonWhitespaceCharacter,
            encodingBeforeTrim.begin() + lastNonWhitespaceCharacter + 1
        );
        Utf8::Utf8 utf8;
        const auto encodingUtf8 = utf8.Encode(encoding);
        json.impl_->encoding = std::string(
            encodingUtf8.begin(),
            encodingUtf8.end()
        );
        if (encoding.empty()) {
        } else if (
            (encoding[0] == '{')
            && (encoding[encoding.size() - 1] == '}')
        ) {
            json.impl_->ParseAsObject(
                std::vector< Utf8::UnicodeCodePoint >(
                    encoding.begin() + 1,
                    encoding.begin() + encoding.size() - 1
                )
            );
        } else if (
            (encoding[0] == '[')
            && (encoding[encoding.size() - 1] == ']')
        ) {
            json.impl_->ParseAsArray(
                std::vector< Utf8::UnicodeCodePoint >(
                    encoding.begin() + 1,
                    encoding.begin() + encoding.size() - 1
                )
            );
        } else if (
            (encoding[0] == '"')
            && (encoding[encoding.size() - 1] == '"')
        ) {
            std::string output;
            Utf8::Utf8 utf8;
            const auto utf8EncodedString = utf8.Encode(
                std::vector< Utf8::UnicodeCodePoint >(
                    encoding.begin() + 1,
                    encoding.begin() + encoding.size() - 1
                )
            );
            if (
                DecodeString(
                    std::string(
                        utf8EncodedString.begin(),
                        utf8EncodedString.end()
                    ),
                    output
                )
            ) {
                json.impl_->type = Type::String;
                json.impl_->stringValue = new std::string(output);
            }
        } else if (json.impl_->encoding == "null") {
            json.impl_->type = Type::Null;
        } else if (json.impl_->encoding == "true") {
            json.impl_->type = Type::Boolean;
            json.impl_->booleanValue = true;
        } else if (json.impl_->encoding == "false") {
            json.impl_->type = Type::Boolean;
            json.impl_->booleanValue = false;
        } else {
            if (json.impl_->encoding.find_first_of("+.eE") != std::string::npos) {
                json.impl_->DecodeAsFloatingPoint(encoding);
            } else {
                json.impl_->DecodeAsInteger(encoding);
            }
        }
        return json;
    }

    Json Json::FromEncoding(const std::string& encodingBeforeTrim) {
        Utf8::Utf8 decoder;
        return FromEncoding(decoder.Decode(encodingBeforeTrim));
    }

    Json JsonArray(std::initializer_list< const Json > args) {
        Json json(Json::Type::Array);
        for (
            auto arg = args.begin();
            arg != args.end();
            ++arg
        ) {
            json.Add(*arg);
        }
        return json;
    }

    Json JsonObject(std::initializer_list< std::pair< const std::string, const Json > > args) {
        Json json(Json::Type::Object);
        for (
            auto arg = args.begin();
            arg != args.end();
            ++arg
        ) {
            json.Set(arg->first, arg->second);
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
