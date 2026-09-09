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

#include "G_local.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "Prefabs.h"
#include "Playground.h"

char* spawnVars[MAX_SPAWN_VARS][2];
int numSpawnVars = 0;

bool G_SpawnString( const char *key, const char *defaultString, char **out ) {
	int		i;

	for (int i = 0; i < MAX_SPAWN_VARS && spawnVars[i][0] && spawnVars[i][1]; i++) {
		if (!strcmp(spawnVars[i][0], key)) {
			*out = spawnVars[i][1];
			return true;
		}
	}

	*out = (char *)defaultString;
	return false;
}

bool	G_SpawnFloat( const char *key, const char *defaultString, float *out ) {
	char		*s;
	bool	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atof( s );
	return present;
}

bool 	G_SpawnInt( const char *key, const char *defaultString, int *out ) {
	char		*s;
	bool	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atoi( s );
	return present;
}

bool 	G_SpawnVector( const char *key, const char *defaultString, float *out ) {
	char 		*s;
	bool 	present;

	present = G_SpawnString(key, defaultString, &s);

	sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] );
	return present;
}

void SP_worldspawn()
{
	if (!g_playground) return;
	Resource_key texkey = (Resource_key){0};
	Resource_key modkey = loadModelResource("\\map");
	int obid = g_playground->addObject(g_playground, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f, 1.0f, 1.0f}, &texkey, &modkey, OBJ_NONE );

	
}

void SP_info_player_start()
{
	

	if (!g_playground) return;
	Resource_key key = (Resource_key){0};
	float pos[3] = {0.0f, 0.0f, 0.0f};
	float angle = 0.0f;
	G_SpawnVector("origin", "0 0 0", pos);
	G_SpawnFloat("angle", "0", &angle);
	int obid = g_playground->addObject(g_playground, (Vector3){-pos[0]*MULTIPLIER, pos[2]*MULTIPLIER, pos[1]*MULTIPLIER}, (Vector3){0.0f, 0.0f, 0.0f}, &key, &key, OBJ_NONE );
	Object* ob = &(g_playground->objects[obid]);
	character_create(ob, &g_playground->camera, g_playground);

	g_playground->camera.setParent(&g_playground->camera, ob);
	CameraData* camdata = (CameraData*)g_playground->camera.data;

	camdata->yaw = (270.0f - angle)*DEG2RAD;
	
}


spawn_t	spawns[] = {
	// info entities don't do anything at all, but provide positional
	// information for things controlled by other processes
	{"worldspawn", SP_worldspawn},
	{"info_player_start", SP_info_player_start},
	

	{0, 0}
};


void G_CallSpawn()
{
	char* classname = NULL;

	if (!G_SpawnString("classname", NULL, &classname)) return;

	for (spawn_t* s = spawns; s->name; s++) {
		if (!strcmp(s->name, classname)) {
			s->spawn();
		}
	}

}