/**
 * @file Json.cpp
 *
 * This module contains the implementation of the Json::Json class.
 *
 * © 2018 by Richard Walters
 */

#include <Json/Json.hpp>
#include <map>
#include <string>
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

    std::string Json::ToString(const EncodingOptions& options) const {
        switch (impl_->type) {
            case Impl::Type::Null: return "null";
            case Impl::Type::Boolean: return impl_->booleanValue ? "true" : "false";
            case Impl::Type::String: return (
                "\""
                + Escape(*impl_->stringValue, options)
                + "\""
            );
            default: return "???";
        }
    }

    Json Json::FromString(const std::string& encoding) {
        if (encoding == "null") {
            return nullptr;
        } else if (encoding == "true") {
            return true;
        } else if (encoding == "false") {
            return false;
        } else if (
            !encoding.empty()
            && (encoding[0] == '"')
            && (encoding[encoding.length() - 1] == '"')
        ) {
            return Unescape(encoding.substr(1, encoding.length() - 2));
        } else {
            return Json();
        }
    }

}
