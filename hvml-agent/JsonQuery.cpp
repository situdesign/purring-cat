// This file is a part of Purring Cat, a reference implementation of HVML.
//
// Copyright (C) 2020, <liuxinouc@126.com>.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "interpreter/str_tools.h"
#include "hvml/hvml_to_string.h"
#include "hvml/hvml_log.h"
#include "JsonQuery.h"
#include <string.h>

JsonQuery::JsonQuery(JsonQuery& in)
: error_(in.error_)
, jo_(in.jo_)
{
    A(jo_, "internal logic error");
}

JsonQuery::JsonQuery(hvml_jo_value_t* jo)
: error_(false)
, jo_(jo)
{
    A(jo_, "internal logic error");
}

JsonQuery& JsonQuery::find(const char* query_s)
{
    if (error_) {
        return *this;
    }

    switch (hvml_jo_value_type(jo_)) {

        case MKJOT(J_TRUE): {
            error_ = true;
        } break;

        case MKJOT(J_FALSE): {
            error_ = true;
        } break;

        case MKJOT(J_NULL): {
            error_ = true;
        } break;

        case MKJOT(J_NUMBER): {
            error_ = true;
        } break;

        case MKJOT(J_STRING): {
            error_ = true;
        } break;

        case MKJOT(J_OBJECT): {
            hvml_jo_value_t *child = hvml_jo_value_child(jo_);
            while (child) {
                JsonQuery jq(child);
                jq.find(query_s);
                if (! jq.error()) {
                    jo_ = jq.jo_;
                    return *this;
                }
                child = hvml_jo_value_sibling_next(child);
            }
            error_ = true;
        } break;

        case MKJOT(J_OBJECT_KV): {
            const char *key;
            hvml_jo_value_t* val;
            A(0==hvml_jo_kv_get(jo_, &key, &val), "internal logic error");
            A(key, "internal logic error");
            if (0 == strcmp(query_s, key)) {
                jo_ = val;
            }
            else {
                error_ = true;
            }
        } break;

        case MKJOT(J_ARRAY): {
            hvml_jo_value_t *child = hvml_jo_value_child(jo_);
            while (child) {
                JsonQuery jq(child);
                jq.find(query_s);
                if (! jq.error()) {
                    jo_ = jq.jo_;
                    return *this;
                }
                child = hvml_jo_value_sibling_next(child);
            }
            error_ = true;
        } break;

        default: {
            A(0, "print json type [%d]: not implemented yet", hvml_jo_value_type(jo_));
        } break;
    }

    return *this;
}

JsonQuery& JsonQuery::head()
{
    if (error_) {
        return *this;
    }

    hvml_jo_value_t *child = hvml_jo_value_child(jo_);
    if (child) {
        jo_ = child;
        return *this;
    }

    error_ = true;
    return *this;
}

JsonQuery& JsonQuery::next()
{
    if (error_) {
        return *this;
    }

    hvml_jo_value_t *child = hvml_jo_value_sibling_next(jo_);
    if (child) {
        jo_ = child;
        return *this;
    }

    error_ = true;
    return *this;
}

hvml_jo_value_t* JsonQuery::get()
{
    if (error_) {
        return NULL;
    }
    return jo_;
}

hvml_string_t JsonQuery::getString()
{
    if (error_) {
        return create_string("error");
    }

    if (hvml_jo_value_type(jo_) == MKJOT(J_STRING)) {
        const char* s;
        if (0 == hvml_jo_string_get(jo_, &s)) {
            return create_string(s);
        }
    }

    return hvml_jo_value_to_string(jo_);
}

int64_t JsonQuery::getInt()
{
    if (error_) {
        return -99999999;
    }

    A(hvml_jo_value_type(jo_) == MKJOT(J_NUMBER), "internal logic error");

    int is_integer;
    int64_t i;
    double d;
    const char *s;
    A(0==hvml_jo_number_get(jo_, &is_integer, &i, &d, &s), "internal logic error");

    if (is_integer) {
        return i;
    } else {
        return (int64_t)d;
    }
}

double JsonQuery::getDouble()
{
    if (error_) {
        return -99999999.99;
    }

    A(hvml_jo_value_type(jo_) == MKJOT(J_NUMBER), "internal logic error");

    int is_integer;
    int64_t i;
    double d;
    const char *s;
    A(0==hvml_jo_number_get(jo_, &is_integer, &i, &d, &s), "internal logic error");

    if (is_integer) {
        return (double)i;
    } else {
        return d;
    }
}

bool JsonQuery::setString(const char* json_s)
{
    if (error_) {
        return false;
    }

    if (jo_) {
        hvml_jo_set_string(jo_, json_s, strlen(json_s));
    }
    return true;
}

bool JsonQuery::set(hvml_jo_value_t* jo)
{
    if (error_) {
        return false;
    }
    // unfinished
    return true;
}

bool JsonQuery::enumerable()
{
    if (error_) {
        return false;
    }

    switch (hvml_jo_value_type(jo_)) {
        case MKJOT(J_OBJECT):
        case MKJOT(J_ARRAY): {
            return true;
        }
    }
    return false;
}