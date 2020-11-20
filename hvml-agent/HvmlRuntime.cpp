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

#include "hvml/hvml_string.h"
#include "interpreter/str_tools.h"
#include "HvmlRuntime.h"
#include "JsonQuery.h"

#include <iostream>
#include <exception>
#include <algorithm> // for_each

HvmlRuntime::HvmlRuntime(FILE *hvml_in_f)
: m_vdom(NULL)
, m_udom(NULL)
{
    m_vdom = hvml_dom_load_from_stream(hvml_in_f);
    GetRuntime(m_vdom,
               &m_udom,
               &m_mustache_part,
               &m_archetype_part,
               &m_iterate_part,
               &m_init_part,
               &m_observe_part);
}

HvmlRuntime::~HvmlRuntime()
{
    if (m_vdom) hvml_dom_destroy(m_vdom);
    if (m_udom) hvml_dom_destroy(m_udom);
    m_mustache_part.clear();
    m_archetype_part.clear();
    m_iterate_part.clear();
    m_init_part.clear();
    m_observe_part.clear();
}


const char html_filename[] = "index/index.html";

size_t HvmlRuntime::GetIndexResponse(char* response,
                                     size_t response_limit)
{
    if (! m_udom) return 0;

    FILE *out = fopen(html_filename, "wb+");
    if (! out) {
        E("failed to create output file: %s", html_filename);
        
        return 0;
    }

    DumpUdomPart(m_udom, out);

    fseek(out, 0, SEEK_SET);
    size_t ret_len = fread(response, 1, response_limit, out);
    response[ret_len] = '\0';
    fclose(out);
    return ret_len;
}

bool HvmlRuntime::Refresh(void)
{
    try {
        TransformMustacheGroup();
        TransformArchetypeGroup();
        TransformIterateGroup();
        TransformObserveGroup();
    }
    catch (std::exception& e) { 
        std::cout << e.what() << '\n';
        return false;
    }
    return true;
}

void HvmlRuntime::TransformMustacheGroup()
{
    for_each(m_mustache_part.begin(),
             m_mustache_part.end(),
             [this](mustache_t& item)->void{

                hvml_string_t s = {NULL, 0};
                hvml_string_t s_replaced = {NULL, 0};

                if (GetDollarString(item.s_inner_str.str, &s)) {

                    hvml_dom_t *udom = item.udom;
                    switch (hvml_dom_type(udom))
                    {
                        case MKDOT(D_ATTR): {
                            const char *val = hvml_dom_attr_val(udom);
                            I("+++ val: %s", val);
                            I("+++ mustache: %s", s.str);
                            s_replaced = replace_string(item.s_full_str, s, val);
                            I("+++ s_replaced: %s", s_replaced.str);
                            hvml_dom_attr_set_val(udom, s_replaced.str, s_replaced.len);
                        }
                        break;

                        case MKDOT(D_TEXT): {
                            const char *text = hvml_dom_text(udom);
                            s_replaced = replace_string(item.s_full_str, s, text);
                            hvml_dom_set_text(udom, s_replaced.str, s_replaced.len);
                        }
                        break;
                    }

                    hvml_string_clear(&s);
                    hvml_string_clear(&s_replaced);
                }
             });
}

void HvmlRuntime::TransformArchetypeGroup()
{
    ;
}

void HvmlRuntime::TransformIterateGroup()
{
    ;
}

void HvmlRuntime::TransformObserveGroup()
{
    ;
}

hvml_dom_t* HvmlRuntime::FindInitData(hvml_string_t as_s)
{
    InitGroup_t::iterator it = m_init_part.begin();
    for (; it != m_init_part.end(); it ++) {
        if (0 == strcmp(it->s_as.str, as_s.str)) {
            return it->vdom;
            break;
        }
    }
    return NULL;
}

// dollar_s : "$capital_list.shandong"
// output_s : "ji-nan"
// 
// <init as = "capital_list">
//  [
//    "shandong": "ji-nan",
//    "jiangsu":  "nanjing"
//  ]
// </init>
// 
// This function has not completed.
bool HvmlRuntime::GetDollarString(const char* dollar_s,
                                  hvml_string_t* output_s)
{
    if ('$' != dollar_s[0]) return false;

    StringArray_t sa;
    int n = split_string(sa, &dollar_s[1], ".");
    if (n < 1) {
        clear_StringArray(sa);
        return false;
    }

    hvml_dom_t* vdom = FindInitData(sa[0]);
    if (! vdom) {
        clear_StringArray(sa);
        return false;
    }

    hvml_jo_value_t* jo = hvml_dom_jo(vdom);
    if (! jo) {
        clear_StringArray(sa);
        return false;
    }

    JsonQuery jq(jo);
    for (int i = 1; i < n; i ++) {
        jq = jq.find(sa[i].str);
    }
    if (jq.error()) {
        clear_StringArray(sa);
        return false;
    }

    hvml_string_t s = jq.getString();
    hvml_string_set(output_s, s.str, s.len);
    return true;
}

bool HvmlRuntime::GetDollarString(const char* dollar_s,
                                  hvml_string_t init_as_s,
                                  hvml_string_t* output_s)
{
    if ('$' != dollar_s[0]) return false;

    hvml_dom_t* vdom = FindInitData(init_as_s);
    if (! vdom) return false;

    hvml_jo_value_t* jo = hvml_dom_jo(vdom);
    if (! jo) return false;

    StringArray_t sa;
    int n = split_string(sa, &dollar_s[1], ".");
    if (n < 1) {
        clear_StringArray(sa);
        return false;
    }

    JsonQuery jq(jo);
    for (int i = 0; i < n; i ++) {
        jq = jq.find(sa[i].str);
    }
    if (jq.error()) {
        clear_StringArray(sa);
        return false;
    }

    hvml_string_t s = jq.getString();
    hvml_string_set(output_s, s.str, s.len);
    return true;
}

bool HvmlRuntime::GetDollarString(hvml_string_t init_as_s,
                                  const char* templet_s,
                                  hvml_string_t* output_s)
{


    // unfinished
    
    return true;
}