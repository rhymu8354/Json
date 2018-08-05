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
#include <stddef.h>
#include <string>
#include <Utf8/Utf8.hpp>
#include <utility>

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

        /**
         * This flag indicates whether or not to add whitespace
         * to line up and indent nested structures when encoding
         * JSON values.
         */
        bool pretty = false;

        /**
         * If pretty printing is enabled, and an element of
         * an array or object is wrapped to a new line,
         * this is the number of spaces to intent it
         * relative to its parent.
         */
        size_t spacesPerIndentationLevel = 4;

        /**
         * If pretty printing is enabled, this is the maximum
         * length to allow for any one line before breaking
         * up a line between elements of an array or object.
         */
        size_t wrapThreshold = 60;

        /**
         * This is the number of levels of nesting under which to
         * assume the JSON value is located.  It's used to
         * compute the actual number of spaces to indent
         * in order to show the value's elements at the correct depth.
         */
        size_t numIndentationLevels = 0;
    };

    /**
     * This class represents a data structure that was parsed from,
     * or can be rendered to, a string in the JavaScript Object
     * Notation (JSON) Data Interchange Format, as specified
     * in RFC 7159 (https://tools.ietf.org/html/rfc7159).
     */
    class Json {
        // Types
    public:
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
            Array,
            Object,
        };

        // Lifecycle management
    public:
        ~Json();
        Json(const Json&);
        Json(Json&&);
        Json& operator=(const Json&);
        Json& operator=(Json&&);

        // Public methods
    public:
        /**
         * This is the default constructor.
         *
         * @param type
         *     This is the type of JSON value to create.
         *
         * @note
         *     Setting the type is only useful for invalid, null,
         *     and mutable (array and object) types.
         */
        Json(Type type = Type::Invalid);

        /**
         * This constructs a JSON value consisting of the "null" literal.
         *
         * @param[in] null
         *     This is the value to wrap in JSON.
         */
        Json(nullptr_t);

        /**
         * This constructs a JSON value consisting of a boolean value.
         *
         * @param[in] value
         *     This is the value to wrap in JSON.
         */
        Json(bool value);

        /**
         * This constructs a JSON value consisting of an integer value.
         *
         * @param[in] value
         *     This is the value to wrap in JSON.
         */
        Json(int value);

        /**
         * This constructs a JSON value consisting of a floating point value.
         *
         * @param[in] value
         *     This is the value to wrap in JSON.
         */
        Json(double value);

        /**
         * This constructs a JSON value consisting of a C string value.
         *
         * @param[in] value
         *     This is the value to wrap in JSON.
         */
        Json(const char* value);

        /**
         * This constructs a JSON value consisting of a C++ string value.
         *
         * @param[in] value
         *     This is the value to wrap in JSON.
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
         * This is the inequality comparison operator.
         *
         * @param[in] other
         *     This is the other JSON object to which to compare this one.
         *
         * @return
         *     An indication of whether or not the two JSON objects are
         *     not equal is returned.
         */
        bool operator!=(const Json& other) const;

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
         * This method returns the type of the JSON value.
         *
         * @return
         *     The type of the JSON value is returned.
         */
        Type GetType() const;

        /**
         * This method returns the size of the JSON value,
         * if it's an array.
         *
         * @return
         *     The size of the JSON value is returned, if it's an array.
         *
         * @retval 0
         *     This is returned if the JSON value isn't an array, or if it's
         *     an empty array.
         */
        size_t GetSize() const;

        /**
         * This methods returns an indication of whether or not the JSON
         * value is an object with an inner value having the given key
         * for a name.
         *
         * @param[in] key
         *     This is the name of the inner value for which to check.
         *
         * @return
         *     An indication of whether or not the JSON
         *     value is an object with an inner value having the given key
         *     for a name is returned.
         */
        bool Has(const std::string& key) const;

        /**
         * This method returns the collection of keys of the values
         * in the JSON object.
         *
         * @return
         *     The collection of keys of the values in the JSON
         *     object is returned.
         *
         *     If the JSON value is not an object, an empty collection
         *     is returned.
         */
        std::vector< std::string > GetKeys() const;

        /**
         * This method returns the element at the given index of the
         * JSON value, if it's an array.
         *
         * @param[in] index
         *     This is the position, relative to the front of the array,
         *     of the element to return.
         *
         * @return
         *     The element at the given index of the JSON value is returned.
         *
         *     If there is no element with the given name
         *     in the JSON value, or if the JSON value isn't an object,
         *     a "null" JSON value is returned.
         */
        const Json& operator[](size_t index) const;

        /**
         * This method returns the element at the given index of the
         * JSON value, if it's an array.
         *
         * @param[in] index
         *     This is the position, relative to the front of the array,
         *     of the element to return.
         *
         * @return
         *     The element at the given index of the JSON value is returned.
         *
         *     If there is no element with the given name
         *     in the JSON value, or if the JSON value isn't an object,
         *     a "null" JSON value is returned.
         */
        const Json& operator[](int index) const;

        /**
         * This method returns the element with the given name in the
         * JSON value, if it's an object.
         *
         * @param[in] key
         *     This is the name of the element to return.
         *
         * @return
         *     The element with the given name in the JSON value is returned.
         *
         *     If there is no element with the given name
         *     in the JSON value, or if the JSON value isn't an object,
         *     a "null" JSON value is returned.
         */
        const Json& operator[](const std::string& key) const;

        /**
         * This method returns the element with the given name in the
         * JSON value, if it's an object.
         *
         * @param[in] key
         *     This is the name of the element to return.
         *
         * @return
         *     The element with the given name in the JSON value is returned.
         *
         *     If there is no element with the given name
         *     in the JSON value, or if the JSON value isn't an object,
         *     a "null" JSON value is returned.
         */
        const Json& operator[](const char* key) const;

        /**
         * This method makes a copy of the given value and places it at
         * the end of the array, if the JSON value is an array.
         *
         * @param[in] value
         *     This is the value to copy to the end of the array.
         */
        void Add(const Json& value);

        /**
         * This method makes a copy of the given value and places it at
         * the given index of the array, if the JSON value is an array.
         * Any values previously at or after this index are moved forward
         * one position.
         *
         * @param[in] value
         *     This is the value to copy into the array.
         *
         * @param[in] index
         *     This is the position into which to copy the given value.
         */
        void Insert(const Json& value, size_t index);

        /**
         * This method makes a copy of the given value and places it
         * in the object under the given key, if the JSON value is an object.
         *
         * @param[in] key
         *     This is the name to assign the value when placed in the object.
         *
         * @param[in] value
         *     This is the value to copy into the object.
         */
        void Set(
            const std::string& key,
            const Json& value
        );

        /**
         * This method removes the value at the given index of the
         * array, if the JSON value is an array.
         *
         * @param[in] index
         *     This is the position of the value to remove.
         */
        void Remove(size_t index);

        /**
         * This method removes the value with the given name in
         * the object, if the JSON value is an object.
         *
         * @param[in] key
         *     This is the name of the value to remove.
         */
        void Remove(const std::string& key);

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
         * @param[in] encodingBeforeTrim
         *     This is the encoding of the JSON object to construct.
         *     It may have whitespace characters in the margins.
         */
        static Json FromEncoding(const std::vector< Utf8::UnicodeCodePoint >& encodingBeforeTrim);

        /**
         * This method returns a new JSON object constructed by parsing
         * the JSON object from the given encoding.
         *
         * @param[in] encodingBeforeTrim
         *     This is the encoding of the JSON object to construct.
         *     It may have whitespace characters in the margins.
         */
        static Json FromEncoding(const std::string& encodingBeforeTrim);

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
     * This constructs a JSON array containing copies of the
     * elements in the given initializer list.
     *
     * @param[in] args
     *     These are the values to copy into the new array.
     *
     * @return
     *     The newly constructed JSON array is returned.
     */
    Json JsonArray(std::initializer_list< const Json > args);

    /**
     * This constructs a JSON object containing copies of the
     * elements in the given initializer list.
     *
     * @param[in] args
     *     These are the values to copy into the new object.
     *
     * @return
     *     The newly constructed JSON object is returned.
     */
    Json JsonObject(std::initializer_list< std::pair< const std::string, const Json > > args);

    /**
     * This is a support function for Google Test to print out
     * a Json::Type value.
     *
     * @param[in] type
     *     This is the Json::Type value to print.
     *
     * @param[in] os
     *     This points to the stream to which to print the
     *     Json::Type value.
     */
    void PrintTo(
        Json::Type type,
        std::ostream* os
    );

    /**
     * This is a support function for Google Test to print out
     * a Json value.
     *
     * @param[in] json
     *     This is the JSON value to print.
     *
     * @param[in] os
     *     This points to the stream to which to print the
     *     Json value.
     */
    void PrintTo(
        const Json& json,
        std::ostream* os
    );

}

#endif /* JSON_JSON_HPP */
