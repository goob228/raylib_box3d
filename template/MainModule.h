#ifndef MAINMODULE_H
#define MAINMODULE_H

#include <flecs.h>
#include <raylib.h>
#include <box3d/box3d.h>


#include "Object.h"
#include "Prefabs.h"

extern ecs_world_t* g_world;





extern ECS_COMPONENT_DECLARE(Rotater);
extern ECS_COMPONENT_DECLARE(Position);
extern ECS_COMPONENT_DECLARE(Drawable);
extern ECS_COMPONENT_DECLARE(DrawFunc);
extern ECS_COMPONENT_DECLARE(EventPtr);

extern ECS_COMPONENT_DECLARE(Object);
extern ECS_COMPONENT_DECLARE(CharacterData);


void UpdateMatrix(ecs_iter_t *it);

void DrawWorld(ecs_iter_t *it);

void MainModuleImport(ecs_world_t* world);

#endif //MAINMODULE_H