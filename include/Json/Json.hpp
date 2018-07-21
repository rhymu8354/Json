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
#include <string>

namespace Json {

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
         * This encodes the JSON object into its string format.
         *
         * @return
         *     The string format of the JSON object is returned.
         */
        std::string ToString() const;

        /**
         * This method returns a new JSON object constructed by parsing
         * the JSON object from the given string.
         *
         * @param[in] encoding
         *     This is the string format of the JSON object to construct.
         */
        static Json FromString(const std::string& encoding);

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

}

#endif /* JSON_JSON_HPP */
