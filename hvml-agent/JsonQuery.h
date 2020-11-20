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

#ifndef _json_query_h_
#define _json_query_h_

#include "hvml/hvml_string.h"
#include "hvml/hvml_jo.h"

class JsonQuery
{
public:
    JsonQuery(JsonQuery& in);
    JsonQuery(hvml_jo_value_t* jo);
    JsonQuery find(const char* query_s);
    hvml_jo_value_t* get();
    hvml_string_t getString();
    int64_t getInt();
    double getDouble();
    bool set(const char* json_s);
    bool set(hvml_jo_value_t* jo);
    bool error() { return error_; }
    bool enumerable();

private:
    hvml_jo_value_t* jo_;
    bool error_;
};

#endif //_json_query_h_