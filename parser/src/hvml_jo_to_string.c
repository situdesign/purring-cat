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
#include "hvml/hvml_json_parser.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define TEMP_BUFFER_LEN  64

typedef struct jo_value_to_string_s        jo_value_to_string_t;
struct jo_value_to_string_s {
    int             lvl;
    hvml_string_t  *out;
    char           *temp_buffer;
};

static int traverse_for_string(hvml_jo_value_t *jo, int lvl, int action, void *arg) {
    jo_value_to_string_t *parg = (jo_value_to_string_t*)arg;
    int breakout = 0;
    hvml_jo_value_t *parent = hvml_jo_value_owner(jo);
    hvml_jo_value_t *prev   = hvml_jo_value_sibling_prev(jo);
    switch (hvml_jo_value_type(jo)) {
        case MKJOT(J_TRUE): {
            A(action==0, "internal logic error");
            if (parent && hvml_jo_value_type(parent)==MKJOT(J_OBJECT_KV)) {
                hvml_string_push(parg->out, ':');
            } else if (prev) {
                hvml_string_push(parg->out, ',');
            }
            const char temp[] = "true";
            hvml_string_concat(parg->out, temp, sizeof(temp)-1);
        } break;
        case MKJOT(J_FALSE): {
            A(action==0, "internal logic error");
            if (parent && hvml_jo_value_type(parent)==MKJOT(J_OBJECT_KV)) {
                hvml_string_push(parg->out, ':');
            } else if (prev) {
                hvml_string_push(parg->out, ',');
            }
            const char temp[] = "false";
            hvml_string_concat(parg->out, temp, sizeof(temp)-1);
        } break;
        case MKJOT(J_NULL): {
            A(action==0, "internal logic error");
            if (parent && hvml_jo_value_type(parent)==MKJOT(J_OBJECT_KV)) {
                hvml_string_push(parg->out, ':');
            } else if (prev) {
                hvml_string_push(parg->out, ',');
            }
            const char temp[] = "null";
            hvml_string_concat(parg->out, temp, sizeof(temp)-1);
        } break;
        case MKJOT(J_NUMBER): {
            A(action==0, "internal logic error");
            if (parent && hvml_jo_value_type(parent)==MKJOT(J_OBJECT_KV)) {
                hvml_string_push(parg->out, ':');
            } else if (prev) {
                hvml_string_push(parg->out, ',');
            }
            int is_integer;
            int64_t i;
            double d;
            const char *s;
            size_t len;
            A(0==hvml_jo_number_get(jo, &is_integer, &i, &d, &s), "internal logic error");
            if (is_integer) {
                len = snprintf(parg->temp_buffer, TEMP_BUFFER_LEN, "%" PRId64 "", i);
                hvml_string_concat(parg->out, parg->temp_buffer, len);
            } else {
                int prec = strlen(s);
                len = snprintf(parg->temp_buffer, TEMP_BUFFER_LEN, "%.*g", prec, d);
                hvml_string_concat(parg->out, parg->temp_buffer, len);
            }
        } break;
        case MKJOT(J_STRING): {
            A(action==0, "internal logic error");
            if (parent && hvml_jo_value_type(parent)==MKJOT(J_OBJECT_KV)) {
                hvml_string_push(parg->out, ':');
            } else if (prev) {
                hvml_string_push(parg->out, ',');
            }
            const char *s;
            if (!hvml_jo_string_get(jo, &s)) {
                //hvml_json_str_to_string(parg->out, s, s ? strlen(s) : 0);
                hvml_string_concat(parg->out, s, strlen(s));
            }
        } break;
        case MKJOT(J_OBJECT): {
            switch (action) {
                case 1: {
                    if (parent && hvml_jo_value_type(parent)==MKJOT(J_OBJECT_KV)) {
                        hvml_string_push(parg->out, ':');
                    } else if (prev) {
                        hvml_string_push(parg->out, ',');
                    }
                    hvml_string_push(parg->out, '{');// "}"
                } break;
                case -1: {
                    // "{"
                    hvml_string_push(parg->out, '}');
                } break;
                default: {
                    A(0, "internal logic error");
                } break;
            }
        } break;
        case MKJOT(J_OBJECT_KV): {
            switch (action) {
                case 1: {
                    if (prev) {
                        hvml_string_push(parg->out, ',');
                    }
                    const char      *key;
                    A(0==hvml_jo_kv_get(jo, &key, NULL), "internal logic error");
                    A(key, "internal logic error");
                    A(parent && hvml_jo_value_type(parent)==MKJOT(J_OBJECT), "internal logic error");
                    //hvml_json_str_to_string(parg->out, key, strlen(key));
                    hvml_string_concat(parg->out, key, strlen(key));
                } break;
                case -1: {
                } break;
                default: {
                    A(0, "internal logic error");
                } break;
            }
        } break;
        case MKJOT(J_ARRAY): {
            switch (action) {
                case 1: {
                    if (parent && hvml_jo_value_type(parent)==MKJOT(J_OBJECT_KV)) {
                        hvml_string_push(parg->out, ':');
                    } else if (prev) {
                        hvml_string_push(parg->out, ',');
                    }
                    hvml_string_push(parg->out, '[');// "]";
                } break;
                case -1: {
                    // "["
                    hvml_string_push(parg->out, ']');
                } break;
                default: {
                    A(0, "internal logic error");
                } break;
            }
        } break;
        default: {
            A(0, "print json type [%d]: not implemented yet", hvml_jo_value_type(jo));
        } break;
    }
    return 0;
}

hvml_string_t hvml_jo_value_to_string(hvml_jo_value_t *jo) {
    hvml_string_t ret_s = {NULL, 0};
    jo_value_to_string_t parg;
    parg.lvl = -1;
    parg.out = &ret_s;
    parg.temp_buffer = malloc(TEMP_BUFFER_LEN);
    hvml_jo_value_traverse(jo, &parg, traverse_for_string);
    free(parg.temp_buffer);
    return ret_s;
}
