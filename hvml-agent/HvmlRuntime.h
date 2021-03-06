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

#ifndef _hvml_runtime_h_
#define _hvml_runtime_h_

#include "interpreter/interpreter_runtime.h"
#include "interpreter/str_tools.h"
#include "JsonQuery.h"

#define URL_LEN_MAX 256
class HvmlRuntime : public Interpreter_Runtime
{
public:
    HvmlRuntime(FILE *hvml_in_f, int listen_port);
    ~HvmlRuntime();
    int port() { return listen_port_; }
    const char* url_index() { return url_base_; };
    size_t GetIndexResponse(const char* request,
                            char* response,
                            size_t response_limit);

private:
    int listen_port_;
    char url_base_[URL_LEN_MAX];
    hvml_dom_t *m_vdom; // origin hvml dom
    hvml_dom_t *m_udom; // dom for display
    MustacheGroup_t  m_mustache_part;
    ArchetypeGroup_t m_archetype_part;
    IterateGroup_t   m_iterate_part;
    InitGroup_t      m_init_part;
    ObserveGroup_t   m_observe_part;

private:
    bool Refresh(void);
    int  RebuildHtml(char* response, size_t response_limit);

    void TransformMustacheGroup();
    void TransformIterateGroup();
    void TransformObserveGroup();

    hvml_string_t FindArchetypeTemplet(const char* id_s);
    hvml_dom_t* FindInitData(const char* as_s);

    bool GetDollarString(const char* dollar_s,
                         hvml_string_t* output_s);
    bool GetDollarString(const char* dollar_s,
                         hvml_string_t init_as_s,
                         hvml_string_t* output_s);
    bool GetDollarString(hvml_string_t init_as_s,
                         const char* templet_s,
                         hvml_string_t* output_s);

    bool AppendReplacedDollarTemplet(JsonQuery jq,
                                     StringArray_t& dollars,
                                     StringArray_t& dollars_orign,
                                     const char* templet_s,
                                     hvml_string_t* out);

    bool ObserveProc(int obv_index, StringArray_t& params);
};

#endif //_hvml_runtime_h_