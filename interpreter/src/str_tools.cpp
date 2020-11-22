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
#include "interpreter/ext_tools.h"
#include "interpreter/str_tools.h"
#include <algorithm> // for_each

hvml_string_t create_string(const char* str) {
    hvml_string_t ret_s = {NULL, 0};
    hvml_string_set(&ret_s, str, strlen(str));
    return ret_s;
}

hvml_string_t create_trim_string(const char* str, size_t len) {
    hvml_string_t ret_s = {NULL, 0};
    hvml_string_set(&ret_s, str, len);
    str_trim(ret_s.str);
    ret_s.len = strlen(ret_s.str);
    return ret_s;
}

void clear_StringArray(StringArray_t& sa) {
    for_each(sa.begin(),
             sa.end(),
             [](hvml_string_t& item)->void{
                 hvml_string_clear(&item);
             });
}

void dump_StringArray(StringArray_t& sa)
{
    I(">>> size: %d\n", sa.size());

    for_each(sa.begin(),
             sa.end(),
             [](hvml_string_t& item)->void{
                 I(">>> %s\n", item.str);
             });
    I(">>> -------------end\n");
}

size_t split_string(StringArray_t& sa,
                    const char* src_s,
                    const char* delimiter_s)
{
    int dn = strlen(delimiter_s);
    int n = strlen(src_s);
    const char* s = src_s;
    sa.clear();
    if (dn <= 0) {
        sa.push_back(create_trim_string(s, n));
        return sa.size();
    }
    while (s < (src_s + n)) {
        char* p = strstr((char *)s, delimiter_s);
        if (! p) {
            sa.push_back(create_trim_string(s, n));
            return sa.size();
        }
        sa.push_back(create_trim_string(s, (p-s)));
        s = (p + dn);
    }
    return sa.size();
}

size_t find_all_dollars(StringArray_t& sa,
                        const char* in)
{
    sa.clear();
    char* s = (char *)in;
    const char* p = NULL;
    size_t n = 0;
    p = find_dollar(s, &n);
    while (p) {
        sa.push_back(create_trim_string(p, n));
        s = (char *)p + n;
        p = find_dollar(s, &n);
    }
    return sa.size();
}
