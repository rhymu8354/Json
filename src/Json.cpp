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
        /**
         * These are the different kinds of values that a JSON object can be.
         */
        enum class Type {
            Invalid,
            Null,
            Boolean,
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
        };
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

    bool Json::operator==(const Json& other) const {
        if (impl_->type != other.impl_->type) {
            return false;
        } else switch(impl_->type) {
            case Impl::Type::Null: return true;
            case Impl::Type::Boolean: return impl_->booleanValue == other.impl_->booleanValue;
            default: return true;
        }
    }

    std::string Json::ToString() const {
        switch (impl_->type) {
            case Impl::Type::Null: return "null";
            case Impl::Type::Boolean: return impl_->booleanValue ? "true" : "false";
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
        } else {
            return Json();
        }
    }

}
