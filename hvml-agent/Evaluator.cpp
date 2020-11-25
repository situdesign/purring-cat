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
#include "hvml/hvml_log.h"
#include "Evaluator.h"
#include <stdlib.h>

static char error_info[256] = {0};
double Evaluator::Parser(const char* val, const char** error)
{
    Evaluator* root = NULL;
    memset (error_info, '\0', sizeof(error_info));

    const char* p = val;
    const char* q = val;
    while (*p) {
        switch (*p) {

            case '+':
            case '-': {
                if (root == NULL) {
                    Evaluator* va = new Evaluator(q, p-q, q-val);
                    root = va;
                }
                else {
                    Evaluator* va = new Evaluator(q, p-q, q-val);
                    if (root->right_) {
                        root->right_->right_ = va;
                    }
                    else {
                        root->right_ = va;
                    }
                }
                Evaluator* op = new Evaluator(*p, p-val);
                op->left_ = root;
                root = op;
                q = p+1;
            } break;

            case 'x':
            case '/':
            case '%': {
                if (root == NULL) {
                    Evaluator* va = new Evaluator(q, p-q, q-val);
                    root = va;
                }
                else {
                    Evaluator* va = new Evaluator(q, p-q, q-val);
                    if (root->right_) {
                        root->right_->right_ = va;
                    }
                    else {
                        root->right_ = va;
                    }
                }
                if (root->type_ == TYPE_VALUE) {
                    Evaluator* op = new Evaluator(*p, p-val);
                    op->left_ = root;
                    root = op;
                }
                else if (root->op_ == OP_ADD
                      || root->op_ == OP_SUB) {
                    Evaluator* op = new Evaluator(*p, p-val);
                    op->left_ = root->right_;
                    root->right_ = op;
                }
                else {
                    Evaluator* op = new Evaluator(*p, p-val);
                    op->left_ = root;
                    root = op;
                }
                q = p+1;
            } break;

            default: {
            }
        }
        p ++;
    }

    if (root) {
        Evaluator* va = new Evaluator(q, p-q, q-val);
        if (root->right_) {
            root->right_->right_ = va;
        }
        else {
            root->right_ = va;
        }

        DumpTree(root, 0);

        double ret = root->getValue();
        delete root;
        if (! error_info[0]) {
            *error = NULL;
            return ret;
        }
        else {
            *error = error_info;
            return 0.0;
        }
    }

    snprintf(error_info, sizeof(error_info), "No effective operator.");
    return 0.0;
}

void Evaluator::DumpTree(Evaluator* e, int lvl)
{
    // 左节点往下走，右节点往右走
    if (e->type_ == TYPE_VALUE) {
        printf("(%s)\n", e->value_buffer_);
    }
    else {
        switch (e->op_) {
            case OP_ADD: printf("<+>"); break;
            case OP_SUB: printf("<->"); break;
            case OP_MUL: printf("<x>"); break;
            case OP_DIV: printf("</>"); break;
            case OP_MOD: printf("<%>"); break;
        }
        if (e->right_) {
            printf(" ---> ");
            DumpTree(e->right_, lvl+1);
        }
        if (e->left_) {
            for (int i = 0; i < lvl; i++) {
                printf("\t");
            }
            printf("|\r\n");
            for (int i = 0; i < lvl; i++) {
                printf("\t");
            }
            DumpTree(e->left_, lvl);
        }
    }
}

void Evaluator::TrimZero(char* val)
{
    size_t n = strlen(val);
    char *p = val+n-1;
    while (p > val) {
        if (*p == '0') {
            *p = '\0';
        }
        else if (*p == '.') {
            *p = '\0';
            break;
        }
        else {
            break;
        }
        p --;
    }
}

Evaluator::Evaluator(const char* val, size_t len, int string_pos)
: type_(TYPE_VALUE)
, string_position_(string_pos)
, left_(NULL)
, right_(NULL)
, op_(OP_NULL)
{
    value_buffer_ = new char[len + 1];
    memcpy (value_buffer_, val, len);
    value_buffer_[len] = '\0';
    I("new Evaluator: %s ========", value_buffer_);
}

Evaluator::Evaluator(char op, int string_pos)
: type_(TYPE_OPERATOR)
, string_position_(string_pos)
, value_buffer_(NULL)
, left_(NULL)
, right_(NULL)
{
    switch (op) {
        case '+': op_ = OP_ADD; break;
        case '-': op_ = OP_SUB; break;
        case 'x': op_ = OP_MUL; break;
        case '/': op_ = OP_DIV; break;
        case '%': op_ = OP_MOD; break;
    }
    I("new Evaluator: %c ========", op);
}

Evaluator::~Evaluator()
{
    if (value_buffer_) {
        delete [] value_buffer_;
        value_buffer_ = NULL;
    }
    if (left_) {
        delete left_;
        left_ = NULL;
    }
    if (right_) {
        delete right_;
        right_ = NULL;
    }
}

double Evaluator::getValue()
{
    if (error_info[0]) { // Return directly if there is an error.
        return 0.0;
    }

    if (type_ == TYPE_VALUE) {
        return atof(value_buffer_);
    }
    else {
        if (! left_) {
            snprintf(error_info, sizeof(error_info),
                "Lack of left value. Position: %d", string_position_);
            return 0.0;
        }
        if (! right_) {
            snprintf(error_info, sizeof(error_info),
                "Lack of left value. Position: %d", string_position_);
            return 0.0;
        }
        double lvalue = left_->getValue();
        double rvalue = right_->getValue();
        switch (op_) {
            case OP_ADD: return lvalue + rvalue;
            case OP_SUB: return lvalue - rvalue;
            case OP_MUL: return lvalue * rvalue;
            case OP_MOD: return (long long)lvalue % (long long)rvalue;
            case OP_DIV: {
                if (rvalue > -0.0000001 && rvalue < 0.0000001) {
                    snprintf(error_info, sizeof(error_info),
                        "Division by zero error. Position: %d", string_position_);
                    return 0.0;
                }
                return lvalue / rvalue;
            }
        }
    }

    snprintf(error_info, sizeof(error_info),
        "Unknown type error occurred. Position: %d", string_position_);
    return 0.0;
}