#ifndef JSON_JSON_HPP
#define JSON_JSON_HPP

/**
 * @file Json.hpp
 *
 * This module declares the Json::Json class.
 *
 * © 2018 by Richard Walters
 */

#include <memory>
#include <ostream>
#include <string>

namespace Json {

    /**
     * This is used to configure various options having to do with
     * encoding a Json object into its string format.
     */
    struct EncodingOptions {
        /**
         * This flag indicates whether or not to escape
         * non-ASCII characters when encoding the JSON
         * object into its string format.
         */
        bool escapeNonAscii = false;

        /**
         * This flag indicates whether or not to disregard
         * any cached encoding when asked to provide
         * an encoding.
         */
        bool reencode = false;
    };

    /**
     * This class represents a data structure that was parsed from,
     * or can be rendered to, a string in the JavaScript Object
     * Notation (JSON) Data Interchange Format, as specified
     * in RFC 7159 (https://tools.ietf.org/html/rfc7159).
     */
    class Json {
        // Lifecycle management
    public:
        ~Json();
        Json(const Json&) = delete;
        Json(Json&&);
        Json& operator=(const Json&) = delete;
        Json& operator=(Json&&);

        // Public methods
    public:
        /**
         * This is the default constructor.
         */
        Json();

        /**
         * This constructs a JSON object consisting of the "null" literal.
         *
         * @param[in] null
         *     This is the object to wrap in JSON.
         */
        Json(nullptr_t);

        /**
         * This constructs a JSON object consisting of a boolean value.
         *
         * @param[in] value
         *     This is the object to wrap in JSON.
         */
        Json(bool value);

        /**
         * This constructs a JSON object consisting of an integer value.
         *
         * @param[in] value
         *     This is the object to wrap in JSON.
         */
        Json(int value);

        /**
         * This constructs a JSON object consisting of a floating point value.
         *
         * @param[in] value
         *     This is the object to wrap in JSON.
         */
        Json(double value);

        /**
         * This constructs a JSON object consisting of a C string value.
         *
         * @param[in] value
         *     This is the object to wrap in JSON.
         */
        Json(const char* value);

        /**
         * This constructs a JSON object consisting of a C++ string value.
         *
         * @param[in] value
         *     This is the object to wrap in JSON.
         */
        Json(const std::string& value);

        /**
         * This is the equality comparison operator.
         *
         * @param[in] other
         *     This is the other JSON object to which to compare this one.
         *
         * @return
         *     An indication of whether or not the two JSON objects are equal
         *     is returned.
         */
        bool operator==(const Json& other) const;

        /**
         * This is the typecast to bool operator for the class.
         *
         * @return
         *     The boolean equivalent of the JSON object is returned.
         *
         * @retval true
         *     This is returned if the JSON object is a boolean and its
         *     value is true.
         *
         * @retval false
         *     This is returned if the JSON object is not a boolean, or
         *     it's a boolean and its value is false.
         */
        operator bool() const;

        /**
         * This is the typecast to C++ string operator for the class.
         *
         * @return
         *     The C++ string equivalent of the JSON object is returned.
         *
         * @retval std::string("")
         *     This is returned if the JSON object is not a string, or
         *     it's a string and its value is the empty string.
         */
        operator std::string() const;

        /**
         * This is the typecast to integer operator for the class.
         *
         * @return
         *     The integer equivalent of the JSON object is returned.
         *
         * @retval 0
         *     This is returned if the JSON object is not an integer, or
         *     it's an integer and its value is zero.
         */
        operator int() const;

        /**
         * This is the typecast to floating-point operator for the class.
         *
         * @return
         *     The floating-point equivalent of the JSON object is returned.
         *
         * @retval 0.0
         *     This is returned if the JSON object is not a floating-point
         *     value, or it's a floating-point value and its value is zero.
         */
        operator double() const;

        /**
         * This encodes the JSON object.
         *
         * @param[in] options
         *     This is used to configure various options having to do with
         *     encoding a Json object into its string format.
         *
         * @return
         *     The encoding of the JSON object is returned.
         */
        std::string ToEncoding(const EncodingOptions& options = EncodingOptions()) const;

        /**
         * This method returns a new JSON object constructed by parsing
         * the JSON object from the given encoding.
         *
         * @param[in] encoding
         *     This is the encoding of the JSON object to construct.
         */
        static Json FromEncoding(const std::string& encoding);

        // Private properties
    private:
        /**
         * This is the type of structure that contains the private
         * properties of the instance.  It is defined in the implementation
         * and declared here to ensure that it is scoped inside the class.
         */
        struct Impl;

        /**
         * This contains the private properties of the instance.
         */
        std::unique_ptr< struct Impl > impl_;
    };

    /**
     * This is a support function for Google Test to print out
     * a Json value.
     *
     * @param[in] json
     *     This is the JSON value to print.
     *
     * @param[in] os
     *     This points to the stream to which to print the
     *     server request state value.
     */
    void PrintTo(
        const Json& json,
        std::ostream* os
    );

}

#endif /* JSON_JSON_HPP */
