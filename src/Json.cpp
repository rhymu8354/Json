/**
 * @file Json.cpp
 *
 * This module contains the implementation of the Json::Json class.
 *
 * © 2018 by Richard Walters
 */

#include <Json/Json.hpp>

namespace Json {

    /**
     * This contains the private properties of a Json instance.
     */
    struct Json::Impl {
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
    }

    bool Json::operator==(const Json& other) const {
        return true;
    }

    std::string Json::ToString() const {
        return "null";
    }

    Json Json::FromString(const std::string& format) {
        return Json();
    }

}
