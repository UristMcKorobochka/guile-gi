// Copyright (C) 2018 Michael L. Gran

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef GIG_CONSTANT_H
#define GIG_CONSTANT_H

#include <girepository.h>
#include <libguile.h>

// *INDENT-OFF*
G_BEGIN_DECLS
// *INDENT-ON*

SCM gig_constant_define(GIConstantInfo *info, SCM defs);
void gig_init_constant(void);

G_END_DECLS
#endif
