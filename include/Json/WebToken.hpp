#pragma once

/**
 * @file WebToken.hpp
 *
 * This module declares the Json::WebToken class.
 *
 * © 2019 by Richard Walters
 */

#include <Json/Value.hpp>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace Json {

    /**
     * This class represents a JSON Web Token (JWT), as specified
     * in RFC 7519 (https://tools.ietf.org/html/rfc7519).
     */
    class WebToken {
        // Lifecycle management
    public:
        ~WebToken() noexcept;
        WebToken(const WebToken&);
        WebToken(WebToken&&) noexcept;
        WebToken& operator=(const WebToken&);
        WebToken& operator=(WebToken&&) noexcept;

        // Public methods
    public:
        /**
         * This constructs a JWT from its encoding.
         *
         * @param[in] encoding
         *     This is the encoding of the JWT to represent.
         */
        explicit WebToken(const std::string& encoding);

        /**
         * This is the equality comparison operator.
         *
         * @param[in] other
         *     This is the other JWT to which to compare this one.
         *
         * @return
         *     An indication of whether or not the two JWTs are equal
         *     is returned.
         */
        bool operator==(const WebToken& other) const;

        /**
         * This is the inequality comparison operator.
         *
         * @param[in] other
         *     This is the other JWT to which to compare this one.
         *
         * @return
         *     An indication of whether or not the two JWTs are
         *     not equal is returned.
         */
        bool operator!=(const WebToken& other) const;

        /**
         * Return the header of the JWT.
         *
         * @return
         *     The header of the JWT is returned.
         */
        Value GetHeader() const;

        /**
         * Return the payload of the JWT.
         *
         * @return
         *     The payload of the JWT is returned.
         */
        Value GetPayload() const;

        /**
         * Return the signature of the JWT.
         *
         * @return
         *     The signature of the JWT is returned.
         */
        std::vector< uint8_t > GetSignature() const;

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
        std::unique_ptr< Impl > impl_;
    };

    /**
     * This is a support function for Google Test to print out
     * a JWT.
     *
     * @param[in] json
     *     This is the JWT to print.
     *
     * @param[in] os
     *     This points to the stream to which to print the
     *     JWT.
     */
    void PrintTo(
        const WebToken& json,
        std::ostream* os
    );

}
