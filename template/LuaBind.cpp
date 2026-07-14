#include "LuaBind.h"

#include <raylib.h>

#include "Playground.h"
#include "EventHandler.h"
#include "Prefabs.h"

/*
lua_getglobal(L, "pg");
	Playground* pg = (Playground*)lua_touserdata(L, -1);
*/


void SolLua::init(sol::state& lua, Playground* pg, EventHandler* eh)
{
	lua.open_libraries(sol::lib::base, sol::lib::io, sol::lib::math, sol::lib::table);

	lua["PLAYGROUND"] = pg;
	lua["EVENTHANDLER"] = eh;

	lua["K_W"] = (int)EH_K_W;
	lua["K_D"] = (int)EH_K_D;
	lua["K_A"] = (int)EH_K_A;
	lua["K_SPACE"] = (int)EH_K_SPACE;
	lua["K_E"] = (int)EH_K_E;

	lua.new_usertype<Spring>("Spring",
		sol::constructors<Spring()>(),

		"setTargetPos", &Spring::setTargetPos,
		"getPos", &Spring::getPos,
		"update", &Spring::update,


		"hertz", &Spring::hertz, 
		"damping",  &Spring::damping		
	);

	lua.new_usertype<Car>("Car",
		sol::constructors<Car()>(),

		"print_name", &Car::update,

		"_springDamping", &Car::_springDamping
	);



	lua["getCarPtr"] = [](sol::this_state ts, int objid)
		{
			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			return std::ref(*((Car*)pg->_objects[objid]));

		};

	lua["addTexture"] = [](sol::this_state ts, std::string path)
		{
		sol::state_view lua(ts);
		Playground* pg = lua["PLAYGROUND"];
		pg->addTexture(path.c_str());
		int idx = pg->_textureCount - 1;
		return idx;
		};

	lua["addModel"] = [](sol::this_state ts, std::string path)
		{
		
		sol::state_view lua(ts);
		Playground* pg = lua["PLAYGROUND"];

		pg->addModel(path.c_str());

		int idx = pg->_modelCount - 1;

		return idx;
		};

	lua["addObject"] = [](sol::this_state ts, int texId, int modelId, ObjectType type, 
		float px, float py, float pz, float sx, float sy, float sz)
		{

		sol::state_view lua(ts);
		Playground* pg = lua["PLAYGROUND"];

		int idx = pg->addObject(Vector3{ px, py, pz }, Vector3{ sx, sy, sz }, texId, modelId, type);		

		return idx;
		};

	lua["convertToCar"] = [](sol::this_state ts, int objid)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			pg->_objects[objid] = Car::create(pg->_objects[objid], pg);

			return;
		};

	lua["convertToWheel"] = [](sol::this_state ts, int objid)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			pg->_objects[objid] = Wheel::create(pg->_objects[objid]);

			return;
		};

	lua["setDamping"] = [](sol::this_state ts, int objid, float damping)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			Car* car = (Car*)pg->_objects[objid];
			car->_springDamping = damping;

			return;
		};

	lua["carSteer"] = [](sol::this_state ts, int objid, float angle)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			Car* car = (Car*)pg->_objects[objid];

			car->steer(angle);

			return;
		};

	lua["carAccelerate"] = [](sol::this_state ts, int objid)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			Car* car = (Car*)pg->_objects[objid];

			car->_accelerating = true;

			return;
		};

	lua["carBrake"] = [](sol::this_state ts, int objid)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			Car* car = (Car*)pg->_objects[objid];

			car->_braking = true;

			return;
		};


	lua["addForceToObj"] = [](sol::this_state ts, int objid, float fx, float fy, float fz)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			Object* obj = pg->_objects[objid];

			b3Body_ApplyForceToCenter(pg->_bodies[obj->_physId], b3Vec3{ fx,fy,fz }, true);

			return;
		};

	lua["isKeyPressed"] = [](sol::this_state ts, int key)
		{

			sol::state_view lua(ts);
			EventHandler* eh = lua["EVENTHANDLER"];

			if (eh->_pressedKeys & key) {
				return true;
			}

			return false;
		};

	lua["isKeyDown"] = [](sol::this_state ts, int key)
		{

			sol::state_view lua(ts);
			EventHandler* eh = lua["EVENTHANDLER"];

			if (eh->_keys & key) {
				return true;
			}

			return false;
		};

	lua["setParent"] = [](sol::this_state ts, int objid, int parid)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			Object* obj = pg->_objects[objid];

			Object* par = pg->_objects[parid];

			obj->setParent(par);

			return;
		};

	lua["setCameraParent"] = [](sol::this_state ts, int parid)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];


			Object* par = pg->_objects[parid];

			pg->_camera.setParent(par);

			return;
		};


	lua["setCarToWheel"] = [](sol::this_state ts, int wheelid, int carid)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			Wheel* wheel = (Wheel*)pg->_objects[wheelid];

			Car* car = (Car*)pg->_objects[carid];

			wheel->_car = car;
			wheel->_parent = (Object*)car;
			wheel->_springLen = car->_springLen;

			car->_wheels[car->_wheelCount] = wheel;
			car->_wheelCount++;

			return;
		};

	lua["rotateObject"] = [](sol::this_state ts, int objid, float rx, float ry, float rz)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			Object* obj = pg->_objects[objid];

			b3Vec3 pos = b3Body_GetPosition(pg->_bodies[obj->_physId]);
			Quaternion r = QuaternionFromEuler(rx * DEG2RAD, ry * DEG2RAD, rz * DEG2RAD);
			b3Quat rot = { {r.x, r.y, r.z},r.w };


			b3Body_SetTransform(pg->_bodies[obj->_physId], pos, rot);
			obj->_rot = r;
			obj->updateMatrix();

			return;
		};

	lua["setMassCenter"] = [](sol::this_state ts, int objid, float mx, float my, float mz)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			Object* obj = pg->_objects[objid];

			b3BodyId bid = pg->_bodies[obj->_physId];

			b3Matrix3 in = b3Body_GetLocalRotationalInertia(bid);

			float mass = b3Body_GetMass(bid);

			b3MassData pidor = { mass, b3Vec3{mx,my,mz}, in };

			b3Body_SetMassData(bid, pidor);

			return;
		};


	lua["setObjectPos"] = [](sol::this_state ts, int objid, float x, float y, float z)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			Object* obj = (Object*)pg->_objects[objid];


			obj->_pos = Vector3{ x, y, z };
			if (obj->_physId != 0) {
				b3Quat rot = b3Body_GetRotation(pg->_bodies[obj->_physId]);
				b3Body_SetTransform(pg->_bodies[obj->_physId], b3Vec3{ x,y,z }, rot);
			}

			obj->updateMatrix();

			return;
		};

	lua["createSpring"] = [](sol::this_state ts)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			for (int i = 1; i < pg->_springCount; i++) {
				if (pg->_springs[i].used == false) {
					pg->_springs[i].used == true;
					return i;
				}
			}

			pg->_springs[pg->_springCount].used == true;
			pg->_springs[pg->_springCount].startPosition = 0.0f;
			pg->_springs[pg->_springCount].startVelocity = 0.0f;
			pg->_springs[pg->_springCount].elapsedTime = 0.0f;

			pg->_springCount++;

			return pg->_springCount-1;
		};

	lua["setSpringValues"] = [](sol::this_state ts, int id, float hertz, float damp)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];

			pg->_springs[id].setHertz(hertz);
			pg->_springs[id].setDamping(damp);

			return;
		};

	lua["setSpringTargetPos"] = [](sol::this_state ts, int id, float pos)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];


			pg->_springs[id].setTargetPos(pos);

			return;
		};
	
	lua["updateSpring"] = [](sol::this_state ts, int id)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];


			pg->_springs[id].update(pg->_targetDeltaTime);

			return;
		};

	lua["getSpringPos"] = [](sol::this_state ts, int id)
		{

			sol::state_view lua(ts);
			Playground* pg = lua["PLAYGROUND"];


			return pg->_springs[id].getPos();
		};
	

	try
	{
		lua.safe_script_file("res/textures.lua");
	}
	catch (const sol::error& e)
	{
		TraceLog(LOG_ERROR, (char*)(e.what()));
		return;
	}

}


int lua_addTexture(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	pg->addTexture(lua_tostring(L, 1));
	int idx = pg->_textureCount - 1;
	lua_pushnumber(L, idx);

	return 1;
}

int lua_addModel(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	pg->addModel(lua_tostring(L, 1));

	int idx = pg->_modelCount - 1;
	lua_pushnumber(L, idx);

	return 1;
}

int lua_addObject(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	int texId = lua_tonumber(L, 1);
	int modelId = lua_tonumber(L, 2);
	ObjectType type = (ObjectType)(int)lua_tonumber(L, 3);
	float px = (float)lua_tonumber(L, 4);
	float py = (float)lua_tonumber(L, 5);
	float pz = (float)lua_tonumber(L, 6);
	float sx = (float)lua_tonumber(L, 7);
	float sy = (float)lua_tonumber(L, 8);
	float sz = (float)lua_tonumber(L, 9);

	int idx = pg->addObject(Vector3{ px, py, pz }, Vector3{ sx, sy, sz }, texId, modelId, type);

	lua_pushnumber(L, idx);

	return 1;
}

int lua_convertToCar(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	int objid = lua_tonumber(L, 1);

	pg->_objects[objid] = Car::create(pg->_objects[objid], pg);

	return 1;
}

int lua_convertToWheel(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	int objid = lua_tonumber(L, 1);

	pg->_objects[objid] = Wheel::create(pg->_objects[objid]);

	return 1;
}

int lua_getObjectPointer(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	int objid = lua_tonumber(L, 1);

	lua_pushlightuserdata(L, pg->_objects[objid]);

	return 1;
}

int lua_getCameraObjPointer(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);


	lua_pushlightuserdata(L, (Object*)(&pg->_camera));

	return 1;
}

int lua_setDamping(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Car* car = (Car*)pg->_objects[(int)lua_tonumber(L, 1)];

	float damping = (float)lua_tonumber(L, 2);

	car->_springDamping = damping;

	return 1;
}

int lua_carSteer(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Car* car = (Car*)pg->_objects[(int)lua_tonumber(L, 1)];

	float angle = (float)lua_tonumber(L, 2);

	car->steer(angle);

	return 1;
}

int lua_carAccelerate(lua_State* L)
{

	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Car* car = (Car*)pg->_objects[(int)lua_tonumber(L, 1)];

	car->_accelerating = true;

	return 1;
}

int lua_carBrake(lua_State* L)
{

	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Car* car = (Car*)pg->_objects[(int)lua_tonumber(L, 1)];

	car->_braking = true;

	return 1;
}

int lua_addForceToObj(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Object* obj = pg->_objects[(int)lua_tonumber(L, 1)];

	float fx = (float)lua_tonumber(L, 2);
	float fy = (float)lua_tonumber(L, 3);
	float fz = (float)lua_tonumber(L, 4);

	b3Body_ApplyForceToCenter(pg->_bodies[obj->_physId], b3Vec3{fx,fy,fz}, true);

	return 1;
}

int lua_isKeyPressed(lua_State* L)
{
	lua_getglobal(L, "EVENTHANDLER");
	EventHandler* eh = (EventHandler*)lua_touserdata(L, -1);

	int key = (int)lua_tointeger(L, 1);

	if (eh->_pressedKeys & key) {
		lua_pushboolean(L, 1);
	}
	else {
		lua_pushboolean(L, 0);
	}
		

	return 1;
}

int lua_isKeyDown(lua_State* L)
{
	lua_getglobal(L, "EVENTHANDLER");
	EventHandler* eh = (EventHandler*)lua_touserdata(L, -1);

	int key = (int)lua_tointeger(L, 1);

	if (eh->_keys & key) {
		lua_pushboolean(L, 1);
	}
	else {
		lua_pushboolean(L, 0);
	}


	return 1;
}

int lua_setParent(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Object* obj = pg->_objects[(int)lua_tonumber(L, 1)];

	Object* par = pg->_objects[(int)lua_tonumber(L, 2)];

	obj->setParent(par);

	return 1;
}

int lua_setCameraParent(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Object* par = pg->_objects[(int)lua_tonumber(L, 1)];

	pg->_camera.setParent(par);

	return 1;
}

int lua_setCarToWheel(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Wheel* wheel = (Wheel*)pg->_objects[(int)lua_tonumber(L, 1)];

	Car* car = (Car*)pg->_objects[(int)lua_tonumber(L, 2)];


	wheel->_car = car;
	wheel->_parent = (Object*)car;
	wheel->_springLen = car->_springLen;

	car->_wheels[car->_wheelCount] = wheel;
	car->_wheelCount++;

	return 1;
}

int lua_setIdToWheel(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Wheel* wheel = (Wheel*)pg->_objects[(int)lua_tonumber(L, 1)];

	int id = (int)lua_tonumber(L, 2);


	return 1;
}

int lua_setMassCenter(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Object* obj = (Object*)pg->_objects[(int)lua_tonumber(L, 1)];

	float mx = (float)lua_tonumber(L, 2);
	float my = (float)lua_tonumber(L, 3);
	float mz = (float)lua_tonumber(L, 4);


	b3BodyId bid = pg->_bodies[obj->_physId];

	b3Matrix3 in = b3Body_GetLocalRotationalInertia(bid);

	float mass = b3Body_GetMass(bid);

	b3MassData pidor = { mass, b3Vec3{mx,my,mz}, in };

	b3Body_SetMassData(bid, pidor);


	return 1;
}

int lua_rotateObject(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Object* obj = pg->_objects[(int)lua_tonumber(L, 1)];

	float rx = (float)lua_tonumber(L, 2);
	float ry = (float)lua_tonumber(L, 3);
	float rz = (float)lua_tonumber(L, 4);

	b3Vec3 pos = b3Body_GetPosition(pg->_bodies[obj->_physId]);
	Quaternion r = QuaternionFromEuler(rx * DEG2RAD, ry * DEG2RAD, rz * DEG2RAD);
	b3Quat rot = { {r.x, r.y, r.z},r.w };


	b3Body_SetTransform(pg->_bodies[obj->_physId], pos, rot);
	obj->_rot = r;
	obj->updateMatrix();

	return 1;
}

int lua_objectSetPos(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	Object* obj = (Object*)pg->_objects[(int)lua_tonumber(L, 1)];

	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);

	obj->_pos = Vector3{x, y, z};
	if (obj->_physId != 0) {
		b3Quat rot = b3Body_GetRotation(pg->_bodies[obj->_physId]);
		b3Body_SetTransform(pg->_bodies[obj->_physId], b3Vec3{x,y,z}, rot);
	}

	obj->updateMatrix();


	return 1;
}

int lua_createSpring(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	for (int i = 1; i < pg->_springCount; i++) {
		if (pg->_springs[i].used == false) {
			pg->_springs[i].used == true;
			lua_pushinteger(L, i);
			return 1;
		}
	}

	pg->_springs[pg->_springCount].used == true;
	pg->_springs[pg->_springCount].startPosition = 0.0f;
	pg->_springs[pg->_springCount].startVelocity = 0.0f;
	pg->_springs[pg->_springCount].elapsedTime = 0.0f;

	lua_pushinteger(L, pg->_springCount);
	pg->_springCount++;

	return 1;
}

int lua_setSpringPos(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	int id = (int)lua_tointeger(L, 1);
	float pos = (float)lua_tonumber(L, 2);

	pg->_springs[id].setPos(pos);


	return 1;
}

int lua_setSpringTargetPos(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	int id = (int)lua_tointeger(L, 1);
	float pos = (float)lua_tonumber(L, 2);

	pg->_springs[id].setTargetPos(pos);

	return 1;
}

int lua_setSpringValues(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	int id = (int)lua_tointeger(L, 1);
	float hertz = (float)lua_tonumber(L, 2);
	float damp = (float)lua_tonumber(L, 3);

	pg->_springs[id].setHertz(hertz);
	pg->_springs[id].setDamping(damp);

	return 1;
}

int lua_updateSpring(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	int id = (int)lua_tointeger(L, 1);

	pg->_springs[id].update(pg->_targetDeltaTime);

	return 1;
}

int lua_getSpringPos(lua_State* L)
{
	lua_getglobal(L, "PLAYGROUND");
	Playground* pg = (Playground*)lua_touserdata(L, -1);

	int id = (int)lua_tointeger(L, 1);

	lua_pushnumber(L, pg->_springs[id].getPos());


	return 1;
}



void Lua::init(lua_State* L, Playground* pg, EventHandler* eh)
{
	luaL_openlibs(L);


	lua_pushlightuserdata(L, pg);
	lua_setglobal(L, "PLAYGROUND");

	lua_pushlightuserdata(L, eh);
	lua_setglobal(L, "EVENTHANDLER");

	lua_pushinteger(L, EH_K_W);
	lua_setglobal(L, "K_W");

	lua_pushinteger(L, EH_K_S);
	lua_setglobal(L, "K_S");

	lua_pushinteger(L, EH_K_D);
	lua_setglobal(L, "K_D");

	lua_pushinteger(L, EH_K_A);
	lua_setglobal(L, "K_A");

	lua_pushinteger(L, EH_K_E);
	lua_setglobal(L, "K_E");

	lua_pushinteger(L, EH_K_SPACE);
	lua_setglobal(L, "K_SPACE");

	lua_register(L, "addTexture", lua_addTexture);
	lua_register(L, "addModel", lua_addModel);
	lua_register(L, "addObject", lua_addObject);
	lua_register(L, "convertToCar", lua_convertToCar);
	lua_register(L, "convertToWheel", lua_convertToWheel);
	lua_register(L, "getObjectPointer", lua_getObjectPointer);
	lua_register(L, "getCameraObjPointer", lua_getCameraObjPointer);
	lua_register(L, "setDamping", lua_setDamping);
	lua_register(L, "setParent", lua_setParent);
	lua_register(L, "setMassCenter", lua_setMassCenter);
	lua_register(L, "setCarToWheel", lua_setCarToWheel);
	lua_register(L, "setIdToWheel", lua_setIdToWheel);
	lua_register(L, "carSteer", lua_carSteer);
	lua_register(L, "carAccelerate", lua_carAccelerate);
	lua_register(L, "carBrake", lua_carBrake);
	lua_register(L, "addForceToObj", lua_addForceToObj);
	lua_register(L, "rotateObject", lua_rotateObject);
	lua_register(L, "isKeyPressed", lua_isKeyPressed);
	lua_register(L, "isKeyDown", lua_isKeyDown);
	lua_register(L, "setCameraParent", lua_setCameraParent);
	lua_register(L, "createSpring", lua_createSpring);
	lua_register(L, "updateSpring", lua_updateSpring);
	lua_register(L, "setSpringPos", lua_setSpringPos);
	lua_register(L, "setSpringTargetPos", lua_setSpringTargetPos);
	lua_register(L, "setSpringValues", lua_setSpringValues);
	lua_register(L, "getSpringPos", lua_getSpringPos);
	lua_register(L, "objectSetPos", lua_objectSetPos);
	


	int r = luaL_dofile(L, "res/textures.lua");

	if (r != LUA_OK) {
		TraceLog(LOG_ERROR, "Lua failed: %s", lua_tostring(L, -1));
		lua_close(L);
	}
}