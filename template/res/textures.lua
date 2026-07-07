-- Pidorases

types = {
	NONE = 0,
	STATIC = 1,
	OBSTACLE = 2,
	PROP = 3,
	ENTITY = 4,
	PLAYER = 5,
	ENEMY = 6,
	PROJECTILE = 7,
	ITEM = 8,
	LEN = 9
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
    car3 = "res/car3.obj",
    fn_fal = "res/FN_FAL.obj",
    wheel = "res/Wheel.obj",
    car1 = "res/Car.obj",
    car2 = "res/Car2.obj",
    car3 = "res/Car3.obj",
    car4 = "res/Car4.obj",
    car5 = "res/Car5.obj",
    car5_taxi = "res/Car5_Taxi.obj",
    car5_police = "res/Car5_Police.obj",
    car6 = "res/Car6.obj",
    car7 = "res/Car7.obj",
    car8 = "res/Car8.obj"
    

}

textures = {}


for key, value in pairs(tex_paths) do
    textures[key] = addTexture(pg, value)
end

models = {}

for key, value in pairs(model_paths) do
    models[key] = addModel(pg, value)
    print(models[key])
end



addObject(pg, textures["car3_red"], models.car3, types.PROP, -3.6, 40.0, -3.6, 1.0, 1.0, 1.0)

addObject(pg, textures["Wood_17"], models.box, types.PROP, 0.0, 10.0, 0.0, 1.0, 1.0, 1.0);

addObject(pg, textures["car3_red"], models.car3, types.PROP,  -0.6, 20.0, -0.6, 1.0, 1.0, 1.0);

addObject(pg, textures["fn_fal_texture"], models.fn_fal, types.PROP, 0.0, 10.0, 6.0, 2.0, 2.0, 2.0);

addObject(pg, textures["car1_blue"], models.car1, types.PROP, 0.0, 10.0, -20.0, 1.0, 1.0, 1.0);

addObject(pg, textures["Wood_17"], models.box, types.OBSTACLE, 0.0, -10.0, 0.0, 50.0, 10.0, 50.0);

addObject(pg, textures["car5_red"], models.car5, types.PROP, -30.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["car5_taxi"], models.car5_taxi, types.PROP, -25.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["car4_orange"], models.car4, types.PROP, -20.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["car1_blue"], models.car1, types.PROP, -15.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["car2_blue"], models.car2, types.PROP, -10.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["car3_red"], models.car3, types.PROP, -5.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["car5_police"], models.car5_police, types.PROP, 0.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["car7_green"], models.car7, types.PROP, 5.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["car8_mail"], models.car8, types.PROP, 10.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["car8_purple"], models.car8, types.PROP, 15.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["car6_mud"], models.car6, types.PROP, 20.0, 10.0, 10.0, 1.0, 1.0, 1.0);
addObject(pg, textures["wheel"], models.wheel, types.PROP, 25.0, 10.0, 10.0, 1.0, 1.0, 1.0);