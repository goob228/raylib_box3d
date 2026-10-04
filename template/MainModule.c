#include "MainModule.h"

#include <flecs.h>
#include <raylib.h>

#include "Object.h"
#include "Camera.h"
#include "SysCvar.h"
#include "SysCmd.h"

extern EventHandler eventhandlers[5];


extern Object camera;

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Rotater);
ECS_COMPONENT_DECLARE(Drawable);
ECS_COMPONENT_DECLARE(DrawFunc);
ECS_COMPONENT_DECLARE(EventPtr);
ECS_COMPONENT_DECLARE(Object);
ECS_COMPONENT_DECLARE(CharacterData);

void CharacterUpdate(ecs_iter_t *it)
{
    Object *p = ecs_field(it, Object, 0);
    CharacterData *pdatas = ecs_field(it, CharacterData, 1);
    Position *poses = ecs_field(it, Position, 2);
    Rotater *rotes = ecs_field(it, Rotater, 3);
    EventPtr *eventptrs = ecs_field(it, EventPtr, 4);

    for (int i = 0; i < it->count; i++) {
        character_update2(p+i, pdatas+i, eventptrs+i, poses+i, rotes+i);
    }
}

void UpdateMatrix(ecs_iter_t *it)
{
    Drawable* draws = ecs_field(it, Drawable, 0);
    Position* poses = ecs_field(it, Position, 1);
    Rotater* rotes = ecs_field(it, Rotater, 2);

    for (int i = 0; i < it->count; i++) {
        ob_updateMatrix3(draws+i, poses+i, rotes+i);
    }

}

void DrawWorld(ecs_iter_t *it)
{
    Drawable* draws = ecs_field(it, Drawable, 0);
    DrawFunc* drawfuncs = ecs_field(it, DrawFunc, 1);


    ((CameraData*)camera.data)->startFrame(&camera);

	if (cv_wireframe.valueb) {
		rlEnableWireMode();
		rlDisableBackfaceCulling();
	}

    for (int i = 0; i < it->count; i++) {
        drawfuncs[i].draw(draws+i);
        
        
    }

    if (cv_wireframe.valueb) {
		rlEnableBackfaceCulling();
		rlDisableWireMode();
	}


    ((CameraData*)camera.data)->endFrame(&camera);
}

void MainModuleImport(ecs_world_t* world)
{
    ECS_MODULE(world, MainModule);

    ECS_COMPONENT_DEFINE(world, Position);
    ECS_COMPONENT_DEFINE(world, Rotater);
    ECS_COMPONENT_DEFINE(world, Drawable);
    ECS_COMPONENT_DEFINE(world, DrawFunc);
    ECS_COMPONENT_DEFINE(world, EventPtr);
    ECS_COMPONENT_DEFINE(world, Object);
    ECS_COMPONENT_DEFINE(world, CharacterData);

    ECS_SYSTEM(world, CharacterUpdate, EcsOnUpdate, Object, CharacterData, Position, Rotater, EventPtr);

    //ECS_SYSTEM(world, CharacterUpdate, EcsOnUpdate, Drawable, Position, Rotater);

}