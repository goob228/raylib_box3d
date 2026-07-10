-- Pidorases

k = {
    W = 0,
    S = 1,
    D = 2,
    A = 3,
    SPACE = 4
}


types = {
	NONE = 0,
    EMPTY = 1,
	STATIC = 2,
	OBSTACLE = 3,
	PROP = 4,
	ENTITY = 5,
	PLAYER = 6,
	ENEMY = 7,
	PROJECTILE = 8,
	ITEM = 9,
	LEN = 10
}

tex_paths = {
    Bricks_06 = "res/Bricks_06-128x128.png",
    Wood_17 = "res/Wood_17-128x128.png",
    fn_fal_texture = "res/FN_FAL_texture.png",
    wheel = "res/wheel.png",
    car3_red = "res/car3_red.png",
    car8_mail = "res/Car8_mail.png",
    car1_blue = "res/car_blue.png",
    car2_blue = "res/car2.png",
    car4_orange = "res/car4_lightorange.png",
    car5_red = "res/car5.png",
    car5_taxi = "res/car5_taxi.png",
    car5_police = "res/car5_police_la.png",
    car6_mud = "res/car6.png",
    car7_green = "res/car7_green.png",
    car8_purple = "res/car8_purple.png"
}

model_paths = {
    box = "res/box.obj",
    box_large = "res/box_large.obj",
    car3 = "res/car3.obj",
    fn_fal = "res/FN_FAL.obj",
    wheel = "res/Wheel_ground.obj",
    car1 = "res/Car.obj",
    car2 = "res/Car2.obj",
    car3 = "res/Car3.obj",
    car4 = "res/Car4.obj",
    car5 = "res/Car5.obj",
    car5_taxi = "res/Car5_Taxi.obj",
    car5_police = "res/Car5_Police_naked.obj",
    car6 = "res/Car6.obj",
    car7 = "res/Car7.obj",
    car8 = "res/Car8.obj"
}

textures = {}


for key, value in pairs(tex_paths) do
    textures[key] = addTexture(value)
end

models = {}

for key, value in pairs(model_paths) do
    models[key] = addModel(value)
    print(models[key])
end





addObject(textures["car3_red"], models.car3, types.PROP, -3.6, 40.0, -3.6, 1.0, 1.0, 1.0)

addObject(textures["Wood_17"], models.box, types.PROP, 0.0, 10.0, 0.0, 1.0, 1.0, 1.0);

addObject(textures["car3_red"], models.car3, types.PROP,  -0.6, 20.0, -0.6, 1.0, 1.0, 1.0);

addObject(textures["fn_fal_texture"], models.fn_fal, types.PROP, 0.0, 10.0, 6.0, 2.0, 2.0, 2.0);

addObject(textures["car1_blue"], models.car1, types.PROP, 0.0, 10.0, -20.0, 1.0, 1.0, 1.0);

addObject(textures["Bricks_06"], models.box_large, types.OBSTACLE, 0.0, -10.0, 0.0, 400.0, 10.0, 400.0);

rotatedGround = addObject(textures["Bricks_06"], models.box_large, types.OBSTACLE, 0.0, -30.0, 0.0, 400.0, 10.0, 400.0);

addObject(textures["car5_red"], models.car5, types.PROP, -30.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(textures["car5_taxi"], models.car5_taxi, types.PROP, -25.0, 10.0, 10.0, 1.0, 1.0, 1.0);
child = addObject(textures["car4_orange"], models.car4, types.EMPTY, -20.0, 30.0, 10.0, 1.0, 1.0, 1.0);
addObject(textures["car1_blue"], models.car1, types.PROP, -15.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(textures["car2_blue"], models.car2, types.PROP, -10.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(textures["car3_red"], models.car3, types.PROP, -5.0, 10.0, 10.0, 1.0, 1.0, 1.0);
car = addObject(textures["car5_police"], models.car5_police, types.PROP, 0.0, 10.0, 20.0, 1.0, 1.0, 1.0);
addObject(textures["car7_green"], models.car7, types.PROP, 5.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(textures["car8_mail"], models.car8, types.PROP, 10.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(textures["car8_purple"], models.car8, types.PROP, 15.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(textures["car6_mud"], models.car6, types.PROP, 20.0, 10.0, 10.0, 1.0, 1.0, 1.0);

--- -1.8

wx = 1.1

wy = 0.6

wheel1 = addObject(textures["wheel"], models.wheel, types.EMPTY, -wx, wy, -1.8, 1.0, 1.0, 1.0);

wheel2 = addObject(textures["wheel"], models.wheel, types.EMPTY, wx, wy, -1.8, 1.0, 1.0, 1.0);

wheel3 = addObject(textures["wheel"], models.wheel, types.EMPTY, -wx, wy, 2.2, 1.0, 1.0, 1.0);

wheel4 = addObject(textures["wheel"], models.wheel, types.EMPTY, wx, wy, 2.2, 1.0, 1.0, 1.0);


convertToWheel(wheel1)
convertToWheel(wheel2)
convertToWheel(wheel3)
convertToWheel(wheel4)

convertToCar(car)


rotateObject(rotatedGround, 0.0, 0.0, 20.0)


--childptr = getObjectPointer(pg, child)

setDamping(car, 1400)

setMassCenter(car, 0.0, 0.2, 0.0)

--setParent(childptr, carptr)

setCameraParent(car)

setCarToWheel(wheel1, car)

setCarToWheel(wheel2, car)

setCarToWheel(wheel3, car)

setCarToWheel(wheel4, car)



function update()

    carSteer(car, 0.0)

    if isKeyPressed(K_SPACE) then
        addForceToObj(car, 2000.0, 0.0, 1000.0)
    end

    if isKeyDown(K_D) then
        carSteer(car, -30.0)
    end

    if isKeyDown(K_A) then
        carSteer(car, 30.0)
    end

    if isKeyDown(K_W) then
        carAccelerate(car)
    end
    
    
    return
end


