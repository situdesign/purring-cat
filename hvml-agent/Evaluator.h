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

#ifndef _evaluator_h_
#define _evaluator_h_

#include "hvml/hvml_string.h"
#include "hvml/hvml_jo.h"

class Evaluator
{
private:
    typedef enum {
        TYPE_OPERATOR,
        TYPE_VALUE
    } Type_en;

    typedef enum {
        OP_NULL,
        OP_ADD, // (+)
        OP_SUB, // (-)
        OP_MUL, // (x)
        OP_DIV, // (/)
        OP_MOD  // (%)
    } Operator_en;

public:
    static double Parser(const char* val, const char** error);
    static void DumpTree(Evaluator* e, int lvl);

public:
    Evaluator(const char* val, size_t len, int string_pos);
    Evaluator(char op, int string_pos);
    ~Evaluator();

private:
    double getValue();
    bool isOp() { return type_ == TYPE_OPERATOR; }

private:
    Type_en type_;
    Evaluator* left_;
    Evaluator* right_;
    Operator_en op_;
    char *value_buffer_;
    int string_position_; // for error_info
};

#endif //_evaluator_h_