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

#include "hvml/hvml_log.h"
#include "JsonQuery.h"

JsonQuery::JsonQuery(hvml_jo_value_t* jo)
: jo_(jo)
{
    A(jo, "internal logic error");
}

JsonQuery& JsonQuery::find(const char* query_s)
{
    return *this;
}

hvml_jo_value_t* JsonQuery::get()
{
    return NULL;
}

const char* JsonQuery::getString()
{
    return NULL;
}

int64_t JsonQuery::getInt()
{
    return 0;
}

bool JsonQuery::set(const char* json_s)
{
    return true;
}

bool JsonQuery::set(hvml_jo_value_t* jo)
{
    return true;
}