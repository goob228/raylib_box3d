#ifndef PLAYGROUND_H
#define PLAYGROUND_H


#include <box3d/box3d.h>

#include "EventHandler.h"
#include "WindowHandler.h"
#include "Camera.h"
#include "Object.h"

#define MAX_BODIES 512
#define MAX_OBJECTS 512



class Playground
{
public:

	void add_stat_box();
	void add_dyn_box();
	void add_dyn_box2();
	void add_dyn_sphere();

	void init();
	void render(WindowHandler* windowhandler);
	void cleanUp();
	void update(EventHandler* eventhandler);

	b3BodyId _bodies[MAX_BODIES];
	Object* _objects[MAX_OBJECTS] = { nullptr };

	int _bodyCount = 1;
	int _objCount = 1;

	GameCamera _camera;
	b3WorldId _worldId;

	Playground() = default;
};

#endif