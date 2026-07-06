#include "Playground.h"

#include <box3d/box3d.h>
#include <rlgl.h>

#include "WindowHandler.h"
#include "EventHandler.h"
#include "Camera.h"
#include "Object.h"



void Playground::add_stat_box()
{
	b3BodyDef groundBodyDef = b3DefaultBodyDef();
	groundBodyDef.position = b3Vec3{ 0.0f, -10.0f, 0.0f };
	b3BodyId groundId = b3CreateBody(_worldId, &groundBodyDef);

	b3BoxHull groundBox = b3MakeBoxHull(50.0f, 10.0f, 50.0f);

	b3ShapeDef groundShapeDef = b3DefaultShapeDef();
	b3CreateHullShape(groundId, &groundShapeDef, &groundBox.base);
	_bodies[_bodyCount] = groundId;
	_bodyCount++;
}

void Playground::add_dyn_box()
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = b3Vec3{ 0.0f, 10.0f, 0.0f };
	b3BodyId bodyId = b3CreateBody(_worldId, &bodyDef);

	b3BoxHull dynamicBox = b3MakeCubeHull(1.0f);

	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.baseMaterial.friction = 0.3f;

	b3CreateHullShape(bodyId, &shapeDef, &dynamicBox.base);
	_bodies[_bodyCount] = bodyId;
	_bodyCount++;
}

void Playground::add_dyn_box2()
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = b3Vec3{ -0.2f, 20.0f, -0.2f };
	b3BodyId bodyId = b3CreateBody(_worldId, &bodyDef);

	b3BoxHull dynamicBox = b3MakeCubeHull(1.0f);

	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.baseMaterial.friction = 0.3f;

	b3CreateHullShape(bodyId, &shapeDef, &dynamicBox.base);
	_bodies[_bodyCount] = bodyId;
	_bodyCount++;
}

void Playground::add_dyn_sphere()
{
	b3BodyDef bodyDef = b3DefaultBodyDef();
	bodyDef.type = b3_dynamicBody;
	bodyDef.position = b3Vec3{ 1.1f, 15.0f, 1.1f };
	b3BodyId bodyId = b3CreateBody(_worldId, &bodyDef);

	b3Sphere sphere = { b3Vec3_zero, 0.5f };

	b3ShapeDef shapeDef = b3DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.baseMaterial.friction = 0.3f;

	b3CreateSphereShape(bodyId, &shapeDef, &sphere);
	_bodies[_bodyCount] = bodyId;
	_bodyCount++;
}

void Playground::init(int targetFPS)
{
	_targetFPS = targetFPS;
	_targetDeltaTime = 1.0f / (float)_targetFPS;

	if (ChangeDirectory("D:/Github/raylib_box3d/template/"))
		TraceLog(LOG_ERROR, "Failed to set custom directory: %s", GetWorkingDirectory()); // FIXME TODO HACK

	_camera.init();

	//"Bricks_06-256x256.png"

	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3{ 0.0f, -10.0f, 0.0f };

	_worldId = b3CreateWorld(&worldDef);

	add_stat_box();
	add_dyn_box();
	add_dyn_sphere();
	add_dyn_box2();
	// "D:\GitHub\raylib\examples\shaders\resources\shaders\glsl330\tiling.fs"
	_basicShader = LoadShader(0, TextFormat("D:/GitHub/raylib/examples/shaders/resources/shaders/glsl330/tiling.fs"));
	float tiling[2] = { 5.0f, 5.0f };
	SetShaderValue(_basicShader, GetShaderLocation(_basicShader, "tiling"), tiling, SHADER_UNIFORM_VEC2);

	_textures[_textureCount] = LoadTexture("res/Bricks_06-256x256.png");
	_textureCount++;

	_textures[_textureCount] = LoadTexture("res/Wood_17-256x256.png");
	_textureCount++;

	_objects[_objCount] = new Object();
	_objects[_objCount]->_model = LoadModel("res/box.obj"); 
	_objects[_objCount]->_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = _textures[1];
	_objects[_objCount]->_model.materials[0].shader = _basicShader;
	_objects[_objCount]->_physId = 2;
	_objects[_objCount]->_type = OBJ_PROP;
	_objCount++;

	_objects[_objCount] = new Object();
	_objects[_objCount]->_model = LoadModel("res/box.obj");  
	_objects[_objCount]->_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = _textures[2];
	_objects[_objCount]->_model.materials[0].shader = _basicShader;
	_objects[_objCount]->_physId = 4;
	_objects[_objCount]->_type = OBJ_PROP;
	_objCount++;


}

void Playground::update(EventHandler* eventhandler)
{
	_camera.update();

	b3World_Step(_worldId, _targetDeltaTime, 2);

	for (int i = 1; i <= _objCount; i++) {
		if (_objects[i] != nullptr) {
			_objects[i]->update(this);
		}
	}
}

void Playground::render(WindowHandler* windowhandler)
{
	_camera.startFrame();
	BeginShaderMode(_basicShader);


	/*
	{
		b3Vec3 position = b3Body_GetPosition(_bodies[2]);
		b3Quat rotation = b3Body_GetRotation(_bodies[2]);

		windowhandler->drawBox(position.x, position.y, position.z, 2.0f);
	}
	*/

	{
		b3Vec3 position = b3Body_GetPosition(_bodies[3]);
		b3Quat rotation = b3Body_GetRotation(_bodies[3]);

		windowhandler->drawSphere(position.x, position.y, position.z, 0.5f);
	}

	for (int i = 1; i <= _objCount; i++) {
		if (_objects[i] != nullptr) {
			_objects[i]->draw(this);
		}
	}



	EndShaderMode();
	_camera.endFrame();
}


void Playground::cleanUp()
{
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (_objects[i] != nullptr) {
			_objects[i]->unLoad();
			delete _objects[i];
			_objects[i] = nullptr;
		}
	}
	
	for (int i = 0; i < MAX_BODIES; i++) {
		if (b3Body_IsValid(_bodies[i])) {
			b3DestroyBody(_bodies[i]);
		}
	}

	for (int i = 0; i < MAX_TEXTURES; i++) {
		if (_textures[i].id != rlGetTextureIdDefault()) rlUnloadTexture(_textures[i].id);
		
	}

	UnloadShader(_basicShader);

}