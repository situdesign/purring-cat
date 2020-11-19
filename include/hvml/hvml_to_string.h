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

#ifndef _hvml_to_string_h_
#define _hvml_to_string_h_

#include "hvml/hvml_dom.h"
#include "hvml/hvml_jo.h"

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

hvml_string_t hvml_dom_to_string(hvml_dom_t *dom);
hvml_string_t hvml_jo_value_to_string(hvml_jo_value_t *jo);

#ifdef __cplusplus
}
#endif

#endif // _hvml_to_string_h_

