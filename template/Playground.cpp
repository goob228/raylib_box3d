#include "Playground.h"

#include <box3d/box3d.h>
#include <raylib.h>
#include <rlgl.h>
#include <lua/lua.hpp>


#include "WindowHandler.h"
#include "EventHandler.h"
#include "Camera.h"
#include "Object.h"

#include "Prefabs.h"




void Playground::add_stat_box()
{
	b3BodyDef groundBodyDef = b3DefaultBodyDef();
	groundBodyDef.position = b3Vec3{ 0.0f, -10.0f, 0.0f };
	b3BodyId groundId = b3CreateBody(_worldId, &groundBodyDef);

	b3BoxHull groundBox = b3MakeCubeHull(10.0f); //b3MakeBoxHull(50.0f, 10.0f, 50.0f);

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
	bodyDef.position = b3Vec3{ -0.6f, 20.0f, -0.6f };
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

int Playground::addObject(Vector3 pos, Vector3 scale, int texId, int modelId, ObjectType type)
{
	_objects[_objCount] = new Object();
	_objects[_objCount]->_scale = scale;
	_objects[_objCount]->_pos = pos;
	_objects[_objCount]->_texId = texId;
	_objects[_objCount]->_modelId = modelId;
	_objects[_objCount]->_type = type;
	_objects[_objCount]->updateMatrix();

	if (type == OBJ_PROP || type == OBJ_OBSTACLE) {
		BoundingBox bb = GetModelBoundingBox(_models[modelId]);

		b3Transform transform = { 0 };
		transform.p.x = (bb.max.x + bb.min.x) * scale.x / 2.0f;
		transform.p.y = (bb.max.y + bb.min.y) * scale.y / 2.0f;
		transform.p.z = (bb.max.z + bb.min.z) * scale.z / 2.0f;
		Quaternion q = QuaternionIdentity();
		transform.q.v.x = q.x;
		transform.q.v.y = q.y;
		transform.q.v.z = q.z;
		transform.q.s = q.w;
		
		b3BodyDef bodyDef = b3DefaultBodyDef();
		if (type == OBJ_PROP)
			bodyDef.type = b3_dynamicBody;
		bodyDef.position = b3Vec3{ pos.x, pos.y, pos.z };
		b3BodyId bodyId = b3CreateBody(_worldId, &bodyDef);

		b3BoxHull dynamicBox = b3MakeBoxHull(	(bb.max.x - bb.min.x) * scale.x / 2.0f,
												(bb.max.y - bb.min.y) * scale.y / 2.0f,
												(bb.max.z - bb.min.z) * scale.z / 2.0f);

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		shapeDef.density = 50.0f;
		shapeDef.baseMaterial.friction = 0.5f;

		b3CreateTransformedHullShape(bodyId, &shapeDef, &dynamicBox.base, transform, b3Vec3{1.0f,1.0f,1.0f});
		_bodies[_bodyCount] = bodyId;
		_objects[_objCount]->_physId = _bodyCount;
		_bodyCount++;
	}

	_objCount += 1;

	return _objCount-1;
	
}

void Playground::delete_object()
{
	int i = _objCount-1;
	if (_objects[i] != nullptr) {
		int e = _objects[i]->_physId;
		if (b3Body_IsValid(_bodies[e])) {
			b3DestroyBody(_bodies[e]);
		}

		_bodyCount--;

		delete _objects[i];
		_objects[i] = nullptr;

		
	}
	_objCount--;
}

int Playground::addTexture(char const * fileName)
{
	_textures[_textureCount] = LoadTexture(fileName);
	_textureCount++;

	return 0;
}

int Playground::addModel(char const* fileName)
{
	_models[_modelCount] = LoadModel(fileName);
	_models[_modelCount].materials[0].shader = _basicShader;
	_modelCount++;

	return 0;
}

void Playground::init(int targetFPS)
{

	_targetFPS = targetFPS;
	_targetDeltaTime = 1.0f / (float)_targetFPS;

	if (ChangeDirectory("D:/Github/raylib_box3d/template/"))
		TraceLog(LOG_ERROR, "Failed to set custom directory: %s", GetWorkingDirectory()); // FIXME TODO HACK

	_camera.init();


	b3WorldDef worldDef = b3DefaultWorldDef();
	worldDef.gravity = b3Vec3{ 0.0f, -10.0f, 0.0f };

	_worldId = b3CreateWorld(&worldDef);

	
	_basicShader = LoadShader(0, "");




}

void Playground::update(EventHandler* eventhandler)
{

	_keys = eventhandler->_keys;
	_pressedKeys = eventhandler->_pressedKeys;
	_mx = eventhandler->_mx;
	_my = eventhandler->_my;

	_camera.update(this);

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
	//DrawPlane(Vector3{ 0.0f, 0.0f, 0.0f }, Vector2{ 32.0f, 32.0f }, LIGHTGRAY); // Draw ground
	//DrawCube(Vector3{ -16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, BLUE);     // Draw a blue wall
	//DrawCube(Vector3{ 16.0f, 2.5f, 0.0f }, 1.0f, 5.0f, 32.0f, LIME);      // Draw a green wall
	//DrawCube(Vector3{ 0.0f, 2.5f, 16.0f }, 32.0f, 5.0f, 1.0f, GOLD);

	/*
	{
		b3Vec3 position = b3Body_GetPosition(_bodies[2]);
		b3Quat rotation = b3Body_GetRotation(_bodies[2]);

		windowhandler->drawBox(position.x, position.y, position.z, 2.0f);
	}
	*/

	{
		b3Vec3 position = b3Body_GetPosition(_bodies[1]);
		b3Quat rotation = b3Body_GetRotation(_bodies[1]);

		windowhandler->drawSphere(position.x, position.y, position.z, 0.5f);
	}

	for (int i = 1; i <= _objCount; i++) {
		if (_objects[i] != nullptr) {
			_objects[i]->draw(this);
		}
	}

	Vector3 spos = Vector3{ 0 };
	Vector3 epos = Vector3{ 0 };

	for (int i = 0; i < MAX_LINES/2-1; i++) {
		spos.x = _lines[i * 2 + 0].x;
		spos.y = _lines[i * 2 + 0].y;
		spos.z = _lines[i * 2 + 0].z;

		epos.x = _lines[i * 2 + 1].x;
		epos.y = _lines[i * 2 + 1].y;
		epos.z = _lines[i * 2 + 1].z;
		
		DrawLine3D(spos, epos, MAROON);

	}


	EndShaderMode();
	_camera.endFrame();

}


void Playground::cleanUp()
{

	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (_objects[i] != nullptr) {
			//_objects[i]->unLoad();
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

	for (int i = 0; i < MAX_MODELS; i++) {
		if (_models[i].meshCount > 0) {
			_models[i].materials[0].shader = Shader{ 0 };
			_models[i].materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = rlGetTextureIdDefault();
			UnloadMaterial(_models[i].materials[0]);
			_models[i].materials[0].maps = NULL;
			UnloadModel(_models[i]);
		}
	}

	UnloadShader(_basicShader);

}