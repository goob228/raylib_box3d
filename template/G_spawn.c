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

#include "Object.h"
#include "Prefabs.h"
#include "MainModule.h"
#include "Camera.h"

extern EventHandler eventhandlers[5];

extern Object camera;

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
	Resource_key texkey = (Resource_key){0};
	Resource_key modkey = loadModelResource("\\map");
	//int obid = pg_addObject(g_playground, (Vector3){0.0f, 0.0f, 0.0f}, (Vector3){1.0f, 1.0f, 1.0f}, &texkey, &modkey, OBJ_NONE );
	//map_create(objects+obid);

	ecs_entity_t e = ecs_entity(g_world, {.name = "Map"});


	ecs_set(g_world, e, Drawable, {
		.texres = texkey,
		.modelres = modkey,
		.transform = MatrixIdentity(),
		.rot = QuaternionIdentity(),
		.pos = {0.0f, 0.0f, 0.0f},
		.scale = {1.0f, 1.0f, 1.0f}
	});

	ecs_set(g_world, e, DrawFunc, {
		.draw = &(map_draw2)
	});


}

void SP_info_player_start()
{
	
	Resource_key key = (Resource_key){0};
	float pos[3] = {0.0f, 0.0f, 0.0f};
	float angle = 0.0f;
	G_SpawnVector("origin", "0 0 0", pos);
	G_SpawnFloat("angle", "0", &angle);
	//int obid = pg_addObject(g_playground, (Vector3){-pos[0]*MULTIPLIER, pos[2]*MULTIPLIER, pos[1]*MULTIPLIER}, (Vector3){0.0f, 0.0f, 0.0f}, &key, &key, OBJ_NONE );
	

	ecs_entity_t e = ecs_entity(g_world, {.name = "Player"});


	ecs_set(g_world, e, Object, {
		.setParent = (&ob_setParent),
		.parent = NULL,
		.transform = MatrixIdentity(),
		.rot = QuaternionIdentity(),
		.pos = (Vector3){-pos[0]*MULTIPLIER, pos[2]*MULTIPLIER, pos[1]*MULTIPLIER}, 
		.scale = (Vector3){0.0f, 0.0f, 0.0f},
		.type = OBJ_NONE,
		.physId = 0,
		.texres = key,
		.modelres = key,	
		.alive = true
		
	});

	ecs_set(g_world, e, Position, {
		.prev = (b3Pos){-pos[0]*MULTIPLIER, pos[2]*MULTIPLIER, pos[1]*MULTIPLIER},
		.curr = (b3Pos){-pos[0]*MULTIPLIER, pos[2]*MULTIPLIER, pos[1]*MULTIPLIER}
	});

	ecs_set(g_world, e, Rotater, {
		.prev = b3Quat_identity,
		.curr = b3Quat_identity
	});

	ecs_set(g_world, e, Drawable, {
		.texres = key,
		.modelres = key,
		.transform = MatrixIdentity(),
		.rot = QuaternionIdentity(),
		.pos = (Vector3){-pos[0]*MULTIPLIER, pos[2]*MULTIPLIER, pos[1]*MULTIPLIER}
	});

	ecs_set(g_world, e, EventPtr, {
		&(eventhandlers[0])
	});

	//Object* ob = &(objects[obid]);
	//character_create(ob, &camera, g_playground);

	//camera.setParent(&camera, ob);

	Object* ob = ecs_get_mut(g_world, e, Object);

	//ob->updateMatrix(ob);

	Position* posik = ecs_get_mut(g_world, e, Position);

	ecs_set(g_world, e, CharacterData, {0});

	CharacterData* chardata = ecs_get_mut(g_world, e, CharacterData);

	character_create2(ob, chardata, posik, &camera);

	CameraData* camdata = (CameraData*)camera.data;

	camdata->yaw = (270.0f - angle)*DEG2RAD;

	camera.setParent(&camera, ob);


	
}


void SP_trigger_always()
{
	Resource_key texkey = (Resource_key){0};
	char* modelname = NULL;
	if (!G_SpawnString("model", "*1", &modelname)) return;// FIXME
	Resource_key modkey = loadModelResource(modelname);
	if (modkey.id == 0) return;


	ecs_entity_t e = ecs_new(g_world);


	ecs_set(g_world, e, Drawable, {
		.texres = texkey,
		.modelres = modkey,
		.transform = MatrixIdentity(),
		.rot = QuaternionIdentity(),
		.pos = {0.0f, 0.0f, 0.0f},
		.scale = {1.0f, 1.0f, 1.0f}
	});

	ecs_set(g_world, e, DrawFunc, {
		.draw = &(map_draw2)
	});
}

spawn_t	spawns[] = {
	// info entities don't do anything at all, but provide positional
	// information for things controlled by other processes
	{"worldspawn", SP_worldspawn},
	{"info_player_start", SP_info_player_start},


	{"func_plat", SP_trigger_always},
	{"func_button", SP_trigger_always},
	{"func_door", SP_trigger_always},
	{"func_static", SP_trigger_always},
	{"func_wall", SP_trigger_always},
	{"func_illusionary", SP_trigger_always},
	{"func_rotating", SP_trigger_always},
	{"func_bobbing", SP_trigger_always},
	{"func_pendulum", SP_trigger_always},
	{"func_train", SP_trigger_always},
	{"func_group", SP_trigger_always},
	{"func_timer", SP_trigger_always},	
	{"func_detail", SP_trigger_always},	

	{"trigger_always", SP_trigger_always},
	{"trigger_multiple", SP_trigger_always},
	{"trigger_push", SP_trigger_always},
	{"trigger_teleport", SP_trigger_always},
	{"trigger_hurt", SP_trigger_always},
	

	{0, 0}
};


void G_CallSpawn()
{
	char* classname = NULL;

	if (!G_SpawnString("classname", NULL, &classname)) return;


	for (spawn_t* s = spawns; s->name; s++) {
		if (!strcmp(s->name, classname)) {
			s->spawn();
			return;
		}
	}

	

}