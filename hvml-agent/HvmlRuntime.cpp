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
#include "hvml/hvml_to_string.h"
#include "HvmlRuntime.h"

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

void HvmlRuntime::TransformIterateGroup()
{
    I("+++ +++ + m_iterate_part size: %d", m_iterate_part.size());
    for_each(m_iterate_part.begin(),
             m_iterate_part.end(),
             [this](iterate_t& item)->void{

                // 如果需要处理其他类型的 with 属性，在这里添加分支
                if ('#' == item.s_with.str[0]) {

                    hvml_string_t templet_s = FindArchetypeTemplet(&item.s_with.str[1]);
                    if (hvml_string_is_empty(&templet_s)) return;

                    I("+++ +++ + templet_s: %s", templet_s.str);

                    hvml_string_t replaced_s = {NULL, 0};

                    if (GetDollarString(item.s_on, templet_s.str, &replaced_s)) {

                        I("+++ +++ + replaced_s: %s", replaced_s.str);

                        hvml_dom_t *uo = item.udom_owner;
                        hvml_dom_t *udom = hvml_dom_append_content(uo,
                                                                replaced_s.str,
                                                                replaced_s.len);
                        A(udom, "internal logic error");
                        item.udom = udom;
                    }
                    hvml_string_clear(&templet_s);
                    hvml_string_clear(&replaced_s);
                }
             });
}

void HvmlRuntime::TransformObserveGroup()
{
    ;
}

hvml_string_t HvmlRuntime::FindArchetypeTemplet(const char* id_s)
{
    ArchetypeGroup_t::iterator it = m_archetype_part.begin();
    for (; it != m_archetype_part.end(); it ++) {
        //I("+++ +++ + it->s_id: %s +++ id_s: %s", it->s_id.str, id_s);
        if (0 == strcmp(it->s_id.str, id_s)) {
            hvml_dom_t *vdom = it->vdom;
            return hvml_dom_to_string(vdom);
        }
    }
    return {NULL, 0};
}

hvml_dom_t* HvmlRuntime::FindInitData(const char* as_s)
{
    InitGroup_t::iterator it = m_init_part.begin();
    for (; it != m_init_part.end(); it ++) {
        if (0 == strcmp(it->s_as.str, as_s)) {
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
    if (n <= 0) {
        return false;
    }

    hvml_dom_t* vdom = FindInitData(sa[0].str);
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

    hvml_dom_t* vdom = FindInitData(init_as_s.str);
    if (! vdom) return false;

    hvml_jo_value_t* jo = hvml_dom_jo(vdom);
    if (! jo) return false;

    StringArray_t sa;
    int n = split_string(sa, &dollar_s[1], ".");
    if (n <= 0) {
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
    I("... ... init_as_s: %s  ", init_as_s.str);
    if ('$' != init_as_s.str[0]) return false;
    hvml_dom_t* vdom = FindInitData(&init_as_s.str[1]);
    if (! vdom) return false;

    hvml_jo_value_t* jo = hvml_dom_jo(vdom);
    if (! jo) return false;

    StringArray_t dollars_orign;
    int n = find_all_dollars(dollars_orign, templet_s);
    if (n <= 0) return false;

    I("... ... find_all_dallors: n=%d -----", n);

    hvml_string_t s_replaced = {NULL, 0};
    JsonQuery jq(jo);
    if (jq.enumerable()) {
        // 可列举的类型采用列表处理方式

        StringArray_t dollars;
        // 复制队列，去掉 $?. 部分
        for_each(dollars_orign.begin(),
                 dollars_orign.end(),
                 [&](hvml_string_t& item)->void{
                     hvml_string_t s = create_string(item.str);
                     remove_substring(&s, "$?.");
                     dollars.push_back(s);
                 });

        //dump_StringArray(dollars);
        //dump_StringArray(dollars_orign);

        JsonQuery jqit = jq.head();
        while (! jqit.error()) {
            AppendReplacedDollarTemplet(jqit, 
                                        dollars,
                                        dollars_orign,
                                        templet_s,
                                        &s_replaced);
            jqit = jqit.next();
        }
        clear_StringArray(dollars);
    }
    else {
        // 不可列举的类型就直接替换，替换不成（例如遇到 $?）就直接报错
        AppendReplacedDollarTemplet(jq,
                                    dollars_orign,
                                    dollars_orign,
                                    templet_s,
                                    &s_replaced);
    }
    
    clear_StringArray(dollars_orign);
    if (hvml_string_is_empty(&s_replaced)) return false;
    *output_s = s_replaced;
    return true;
}

bool HvmlRuntime::AppendReplacedDollarTemplet(JsonQuery jq,
                                              StringArray_t& dollars,
                                              StringArray_t& dollars_orign,
                                              const char* templet_s,
                                              hvml_string_t* out)
{
    hvml_string_t s_replaced = create_string(templet_s);
    int n = dollars.size();
    for (int i = 0; i < n; i ++) {
        StringArray_t sa;
        int size = split_string(sa, dollars[i].str, ".");
        if (size <= 0) continue;

        JsonQuery jqq = jq;
        for (int j = 0; j < size; j ++) {
            I("sa *******  %s\n", sa[j].str);
            jqq = jqq.find(sa[j].str);
        }
        if (jqq.error()) {
            clear_StringArray(sa);
            hvml_string_clear(&s_replaced);
            return false;
        }
        hvml_string_t s = jqq.getString();
        I("jqq *******  %s\n", s.str);
        s_replaced = replace_string(dollars_orign[i],
                                    s,
                                    s_replaced.str);
        hvml_string_clear(&s);
    }
    hvml_string_concat(out, s_replaced.str, s_replaced.len);
    hvml_string_clear(&s_replaced);
    return true;
}