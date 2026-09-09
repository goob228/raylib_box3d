/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// g_local.h -- local definitions for game module

#ifndef G_LOCAL_H
#define G_LOCAL_H

#include "Object.h"

#define INV_MULTI 32.0f
#define MULTIPLIER (1.0f / INV_MULTI)

typedef struct Playground Playground;

extern Playground* g_playground;

#define MAX_SPAWN_VARS 64

extern char* spawnVars[MAX_SPAWN_VARS][2];
extern int numSpawnVars;

typedef struct {
	char* name;
	void	(*spawn)();
} spawn_t;

void G_CallSpawn();


#endif //G_LOCAL_H