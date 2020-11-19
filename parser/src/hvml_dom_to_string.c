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

#include "hvml/hvml_to_string.h"

#include "hvml/hvml_log.h"

#include <stdio.h>
#include <string.h>

#define TEMP_BUFFER_LEN  256

typedef struct dom_to_string_s        dom_to_string_t;
struct dom_to_string_s {
    int             lvl;
    hvml_string_t  *out;
};

static void traverse_for_string(hvml_dom_t *dom, int lvl, int tag_open_close, void *arg, int *breakout) {
    dom_to_string_t *parg = (dom_to_string_t*)arg;
    A(parg, "internal logic error");

    *breakout = 0;

    switch (hvml_dom_type(dom)) {
        case MKDOT(D_TAG):
        {
            switch (tag_open_close) {
                case 1: {
                    const char temp[] = "<%s";
                    const char *name = hvml_dom_tag_name(dom);
                    hvml_string_concat2(parg->out, temp, sizeof(temp)-1,
                                                   name, strlen(name));
                } break;
                case 2: {
                    const char temp[] = "/>";
                    hvml_string_concat(parg->out, temp, sizeof(temp)-1);
                } break;
                case 3: {
                    const char temp[] = ">";
                    hvml_string_concat(parg->out, temp, sizeof(temp)-1);
                } break;
                case 4: {
                    const char head[] = "</";
                    const char *name = hvml_dom_tag_name(dom);
                    const char tail[] = ">";
                    hvml_string_concat3(parg->out, head, sizeof(head)-1,
                                                   name, strlen(name),
                                                   tail, sizeof(tail)-1);
                } break;
                default: {
                    A(0, "internal logic error");
                } break;
            }
            parg->lvl = lvl;
        } break;
        case MKDOT(D_ATTR):
        {
            A(parg->lvl >= 0, "internal logic error");
            const char *key = hvml_dom_attr_key(dom);
            const char *val = hvml_dom_attr_val(dom);
            hvml_string_push(parg->out, ' ');
            hvml_string_concat(parg->out, key, strlen(key));
            if (val) {
                const char head[] = "=\"";
                const char tail[] = "\"";
                hvml_string_concat(parg->out, head, sizeof(head)-1);
                hvml_dom_attr_val_serialize_string(val, strlen(val), parg->out);
                hvml_string_concat(parg->out, tail, sizeof(tail)-1);
            }
        } break;
        case MKDOT(D_TEXT):
        {
            const char *text = hvml_dom_text(dom);
            hvml_dom_str_serialize_string(text, strlen(text), parg->out);
            parg->lvl = lvl;
        } break;
        case MKDOT(D_JSON):
        {
            hvml_string_t json_string = hvml_jo_value_to_string(hvml_dom_jo(dom));
            hvml_string_concat(parg->out, json_string.str, json_string.len);
            parg->lvl = lvl;
        } break;
        default: {
            A(0, "internal logic error");
        } break;
    }
}

hvml_string_t hvml_dom_to_string(hvml_dom_t *dom) {
    hvml_string_t ret_s = {NULL, 0};
    dom_to_string_t parg;
    parg.lvl = -1;
    parg.out = &ret_s;
    hvml_dom_traverse(dom, &parg, traverse_for_string);
    return ret_s;
}
