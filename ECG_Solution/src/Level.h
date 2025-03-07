#pragma once

#include <list>
#include "Hitbox.h"
#include "KomplexStaticBody.h"
#include "Goal.h"
#include "Border.h"
#include "Light.h"
#include <PxPhysicsAPI.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <learnopengl/model_animation.h>
#include <INIReader.h>



class Level {
private:
	bool loaded = false;
public:
	float SCR_WIDTH, SCR_HEIGHT;

	glm::vec3 playerPosition;
	float playerYaw;

	glm::vec3 cameraPosition;

	list <Model> StaticModelList;
	list <Model> ObjectModelList;
	list <Hitbox> BuildingObjects;
	list <KomplexStaticBody> StaticObjects;

	list <Model> goalModel;
	Goal goal;

	list <Model> Background;
	list <Model> Sky;
	list <Model> SkyDay;
	glm::mat4 backgroundM;
	glm::vec3 worldTranslate;
	glm::vec3 worldScale;

	physx::PxPhysics* mPhysics = NULL;

	Border border;

	Light sun;

	int numberOfPointLights = 0;
	list<Light> pointLight;

	int numberOfSpotLights = 0;
	list<Light> Spotlights;

	Level(std::string path, physx::PxPhysics* physics, float width, float heigth) {
		mPhysics = physics;
		SCR_WIDTH = width;
		SCR_HEIGHT = heigth;
		INIReader reader(path);

		float playerX = reader.GetReal("Player", "positionX", 0.0);
		float playerY = reader.GetReal("Player", "positionY", 2.0);
		float playerZ = reader.GetReal("Player", "positionZ", 0.0);
		playerYaw = reader.GetReal("Player", "yaw", 0.0);

		playerPosition = glm::vec3(playerX, playerY, playerZ);

		float cameraX = reader.GetReal("Camera", "positionX", 0.0);
		float cameraY = reader.GetReal("Camera", "positionY", 2.0);
		float cameraZ = reader.GetReal("Camera", "positionZ", 6.0);

		cameraPosition = glm::vec3(cameraX, cameraY, cameraZ);

		int counter = reader.GetInteger("StaticObjects", "tableNumber", 0);
		for (size_t i = 1; i <= counter; i++)
		{
			std::string s = to_string(i);
			float posX = reader.GetReal("StaticObjects", "tablePositionX" + s, 1.0);
			float posY = reader.GetReal("StaticObjects", "tablePositionY" + s, 1.0);
			float posZ = reader.GetReal("StaticObjects", "tablePositionZ" + s, 1.0);
			glm::vec3 pos = glm::vec3(posX, posY, posZ);
			float scaleX = reader.GetReal("StaticObjects", "tableScaleX" + s, 1.0);
			float scaleY = reader.GetReal("StaticObjects", "tableScaleY" + s, 1.0);
			float scaleZ = reader.GetReal("StaticObjects", "tableScaleZ" + s, 1.0);
			glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
			Table(pos, scale);
		}

		counter = reader.GetInteger("StaticObjects", "lampNumber", 0);
		for (size_t i = 1; i <= counter; i++)
		{
			std::string s = to_string(i);
			float posX = reader.GetReal("StaticObjects", "lampPositionX" + s, 0.0);
			float posY = reader.GetReal("StaticObjects", "lampPositionY" + s, 0.0);
			float posZ = reader.GetReal("StaticObjects", "lampPositionZ" + s, 0.0);
			glm::vec3 pos = glm::vec3(posX, posY, posZ);
			float scaleX = reader.GetReal("StaticObjects", "lampScaleX" + s, 1.0);
			float scaleY = reader.GetReal("StaticObjects", "lampScaleY" + s, 1.0);
			float scaleZ = reader.GetReal("StaticObjects", "lampScaleZ" + s, 1.0);
			glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
			Lamp(pos, scale);
		}

		if (!loaded) {
			counter = reader.GetInteger("StaticObjects", "houseNumber", 0);
			for (size_t i = 1; i <= counter; i++)
			{
				std::string s = to_string(i);
				float posX = reader.GetReal("StaticObjects", "housePositionX" + s, 1.0);
				float posY = reader.GetReal("StaticObjects", "housePositionY" + s, 1.0);
				float posZ = reader.GetReal("StaticObjects", "housePositionZ" + s, 1.0);
				glm::vec3 pos = glm::vec3(posX, posY, posZ);
				float scaleX = reader.GetReal("StaticObjects", "houseScaleX" + s, 1.0);
				float scaleY = reader.GetReal("StaticObjects", "houseScaleY" + s, 1.0);
				float scaleZ = reader.GetReal("StaticObjects", "houseScaleZ" + s, 1.0);
				glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
				House(pos, scale);
			}
		}

		counter = reader.GetInteger("StaticObjects", "benchL1Number", 0);
		for (size_t i = 1; i <= counter; i++)
		{
			std::string s = to_string(i);
			float posX = reader.GetReal("StaticObjects", "benchL1PositionX" + s, 1.0);
			float posY = reader.GetReal("StaticObjects", "benchL1PositionY" + s, 1.0);
			float posZ = reader.GetReal("StaticObjects", "benchL1PositionZ" + s, 1.0);
			glm::vec3 pos = glm::vec3(posX, posY, posZ);
			float scaleX = reader.GetReal("StaticObjects", "benchL1ScaleX" + s, 1.0);
			float scaleY = reader.GetReal("StaticObjects", "benchL1ScaleY" + s, 1.0);
			float scaleZ = reader.GetReal("StaticObjects", "benchL1ScaleZ" + s, 1.0);
			glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);

			Bench1(pos, scale);
		}

		counter = reader.GetInteger("StaticObjects", "benchL2Number", 0);
		for (size_t i = 1; i <= counter; i++)
		{
			std::string s = to_string(i);
			float posX = reader.GetReal("StaticObjects", "benchL2PositionX" + s, 1.0);
			float posY = reader.GetReal("StaticObjects", "benchL2PositionY" + s, 1.0);
			float posZ = reader.GetReal("StaticObjects", "benchL2PositionZ" + s, 1.0);
			glm::vec3 pos = glm::vec3(posX, posY, posZ);
			float scaleX = reader.GetReal("StaticObjects", "benchL2ScaleX" + s, 1.0);
			float scaleY = reader.GetReal("StaticObjects", "benchL2ScaleY" + s, 1.0);
			float scaleZ = reader.GetReal("StaticObjects", "benchL2ScaleZ" + s, 1.0);
			glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
			Bench2(pos, scale);
		}

		counter = reader.GetInteger("BuildingObjects", "stone1Number", 0);
		for (size_t i = 1; i <= counter; i++)
		{
			std::string s = to_string(i);
			float posX = reader.GetReal("BuildingObjects", "stone1PositionX" + s, 0.0);
			float posY = reader.GetReal("BuildingObjects", "stone1PositionY" + s, 0.0);
			float posZ = reader.GetReal("BuildingObjects", "stone1PositionZ" + s, 0.0);
			glm::vec3 pos = glm::vec3(posX, posY, posZ);
			float scaleX = reader.GetReal("BuildingObjects", "stone1ScaleX" + s, 0.2);
			float scaleY = reader.GetReal("BuildingObjects", "stone1ScaleY" + s, 0.2);
			float scaleZ = reader.GetReal("BuildingObjects", "stone1ScaleZ" + s, 0.2);
			glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
			Stone1(pos, scale);
		}

		counter = reader.GetInteger("BuildingObjects", "stone2Number", 0);
		for (size_t i = 1; i <= counter; i++)
		{
			std::string s = to_string(i);
			float posX = reader.GetReal("BuildingObjects", "stone2PositionX" + s, 0.0);
			float posY = reader.GetReal("BuildingObjects", "stone2PositionY" + s, 0.0);
			float posZ = reader.GetReal("BuildingObjects", "stone2PositionZ" + s, 0.0);
			glm::vec3 pos = glm::vec3(posX, posY, posZ);
			float scaleX = reader.GetReal("BuildingObjects", "stone2ScaleX" + s, 0.2);
			float scaleY = reader.GetReal("BuildingObjects", "stone2ScaleY" + s, 0.2);
			float scaleZ = reader.GetReal("BuildingObjects", "stone2ScaleZ" + s, 0.2);
			glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
			Stone2(pos, scale);
		}

		counter = reader.GetInteger("BuildingObjects", "stone3Number", 0);
		for (size_t i = 1; i <= counter; i++)
		{
			std::string s = to_string(i);
			float posX = reader.GetReal("BuildingObjects", "stone3PositionX" + s, 0.0);
			float posY = reader.GetReal("BuildingObjects", "stone3PositionY" + s, 0.0);
			float posZ = reader.GetReal("BuildingObjects", "stone3PositionZ" + s, 0.0);
			glm::vec3 pos = glm::vec3(posX, posY, posZ);
			float scaleX = reader.GetReal("BuildingObjects", "stone3ScaleX" + s, 0.2);
			float scaleY = reader.GetReal("BuildingObjects", "stone3ScaleY" + s, 0.2);
			float scaleZ = reader.GetReal("BuildingObjects", "stone3ScaleZ" + s, 0.2);
			glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
			Stone3(pos, scale);
		}

		counter = reader.GetInteger("BuildingObjects", "ladder1Number", 0);
		for (size_t i = 1; i <= counter; i++)
		{
			std::string s = to_string(i);
			float posX = reader.GetReal("BuildingObjects", "ladder1PositionX" + s, 1.0);
			float posY = reader.GetReal("BuildingObjects", "ladder1PositionY" + s, 1.0);
			float posZ = reader.GetReal("BuildingObjects", "ladder1PositionZ" + s, 1.0);
			glm::vec3 pos = glm::vec3(posX, posY, posZ);
			float scaleX = reader.GetReal("BuildingObjects", "ladder1ScaleX" + s, 1.0);
			float scaleY = reader.GetReal("BuildingObjects", "ladder1ScaleY" + s, 1.0);
			float scaleZ = reader.GetReal("BuildingObjects", "ladder1ScaleZ" + s, 1.0);
			glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
			Ladder1(pos, scale);
		}

		counter = reader.GetInteger("BuildingObjects", "crate1Number", 0);
		for (size_t i = 1; i <= counter; i++)
		{
			std::string s = to_string(i);
			float posX = reader.GetReal("BuildingObjects", "crate1PositionX" + s, 1.0);
			float posY = reader.GetReal("BuildingObjects", "crate1PositionY" + s, 1.0);
			float posZ = reader.GetReal("BuildingObjects", "crate1PositionZ" + s, 1.0);
			glm::vec3 pos = glm::vec3(posX, posY, posZ);
			float scaleX = reader.GetReal("BuildingObjects", "crate1ScaleX" + s, 1.0);
			float scaleY = reader.GetReal("BuildingObjects", "crate1ScaleY" + s, 1.0);
			float scaleZ = reader.GetReal("BuildingObjects", "crate1ScaleZ" + s, 1.0);
			glm::vec3 scale = glm::vec3(scaleX, scaleY, scaleZ);
			Crate1(pos, scale);
		}

		float goalX = reader.GetReal("Goal", "positionX", 0.0);
		float goalY = reader.GetReal("Goal", "positionY", 0.0);
		float goalZ = reader.GetReal("Goal", "positionZ", 0.0);
		float goalRadius = reader.GetReal("Goal", "radius", 0.0);
		goal.initGoal(glm::vec3(goalX, goalY, goalZ), goalRadius);

		Model goalM(reader.Get("Goal", "Model", "assets/Obj/Goals/Cupcake.obj"));
		goalModel.push_front(goalM);

		if (!loaded) {
			Model sky(reader.Get("Background", "Sky", "assets/Obj/World/sky.obj"));
			Sky.push_front(sky);

			Model skyDay(reader.Get("Background", "SkyDay", "assets/Obj/World/sky_day.obj"));
			SkyDay.push_front(skyDay);

			float bScaleX = reader.GetReal("Background", "scaleX", 1.0);
			float bScaleY = reader.GetReal("Background", "scaleY", 1.0);
			float bScaleZ = reader.GetReal("Background", "scaleZ", 1.0);


			worldTranslate = glm::vec3(0.0, -1.0, 0.0);
			worldScale = glm::vec3(bScaleX, bScaleY, bScaleZ);

			backgroundM = glm::translate(glm::mat4(1), worldTranslate);
			backgroundM = glm::scale(backgroundM, worldScale);
		}
		
		float borderWidth = reader.GetReal("Background", "borderWidth", 0.0);
		float borderDepth = reader.GetReal("Background", "borderDepth", 0.0);
		float borderLength = reader.GetReal("Background", "borderLength", 0.0);
		float borderScale = reader.GetReal("Background", "borderScale", 1.0);

		Model borderModel(reader.Get("Background", "borderModel", "asstes/Obj/Border/fence.obj"));
		Background.push_back(borderModel);

		border.setBorder(mPhysics, borderWidth, borderDepth, borderLength, borderScale);

		//DirectionalLight
		float lightDirectionX = reader.GetReal("Light", "directionX", 0.0f);
		float lightDirectionY = reader.GetReal("Light", "directionY", 0.0f);
		float lightDirectionZ = reader.GetReal("Light", "directionZ", 0.0f);

		glm::vec3 lightDirection = glm::vec3(lightDirectionX, lightDirectionY, lightDirectionZ);
		
		float DlightColorR = reader.GetReal("Light", "DcolorR", 1.0f);
		float DlightColorG = reader.GetReal("Light", "DcolorG", 1.0f);
		float DlightColorB = reader.GetReal("Light", "DcolorB", 1.0f);

		glm::vec3 DlightColor = glm::vec3(DlightColorR, DlightColorG, DlightColorB);

		sun.setDirectionalLight(DlightColor, lightDirection);

		loaded = true;
	}

	void Table(glm::vec3 position, glm::vec3 scale) {
		//set Hitboxes 1st vec3 = size of HB, 2nd vec3 = position of HB
		StaticHitbox tableTop(mPhysics, 5.75f, 0.025f, 4.25f, glm::vec3(1.0f*8, 4.825f, -1.0f*10));
		StaticHitbox tablelegRB(mPhysics, 0.2f, 2.9f, 0.2f, glm::vec3(3.6f, 2.05f, -13.3f));
		StaticHitbox tablelegLB(mPhysics, 0.2f, 2.9f, 0.2f, glm::vec3(12.4f, 2.05f, -13.3f));
		StaticHitbox tablelegRF(mPhysics, 0.2f, 2.9f, 0.2f, glm::vec3(3.6f, 2.05f, -6.7f));
		StaticHitbox tablelegLF(mPhysics, 0.2f, 2.9f, 0.2f, glm::vec3(12.4f, 2.05f, -6.7f));

		KomplexStaticBody table(tableTop);
		table.addStaticBody(tablelegRB);
		table.addStaticBody(tablelegLB);
		table.addStaticBody(tablelegLF);
		table.addStaticBody(tablelegRF);

		table.TranslateModelMatrix(glm::vec3(8.0, -1.0f, -10.0));

		table.Scale(scale);
		table.Translate(position);

		
		StaticObjects.push_front(table);

		Model ourModel("assets/Obj/StaticObjects/Garden_table.obj");
		StaticModelList.push_front(ourModel);
	}

	void House(glm::vec3 position, glm::vec3 scale) {
		
		StaticHitbox house(mPhysics, 0.05f, 5.0f, 3.0f, glm::vec3(7.35f, 0.0f, 0.25f));
		StaticHitbox doorStep(mPhysics, 0.5f, 0.05f, 1.0f, glm::vec3(7.0f, 0.0f, 0.25f));


		KomplexStaticBody houseM(house);
		houseM.addStaticBody(doorStep);

		houseM.TranslateModelMatrix(position);
		//houseM.Translate(position);
		houseM.Scale(scale);
		


		StaticObjects.push_front(houseM);
		Model houseModel("assets/Obj/StaticObjects/House.obj");

		StaticModelList.push_front(houseModel);
	}

	void Bench1(glm::vec3 position, glm::vec3 scale) {


		StaticHitbox benchL1(mPhysics, 0.1f, 0.025f, 0.6f, position);

		KomplexStaticBody BenchL1(benchL1);

		BenchL1.TranslateModelMatrix(position + glm::vec3(0.0, -0.1, 0.0));
		//houseM.Translate(position);
		BenchL1.Scale(scale);



		StaticObjects.push_front(BenchL1);
		Model bench1("assets/Obj/StaticObjects/bench1.obj");

		StaticModelList.push_front(bench1);
	}

	void Bench2(glm::vec3 position, glm::vec3 scale) {


		StaticHitbox benchL2(mPhysics, 0.6f, 0.025f, 0.1f, position);

		KomplexStaticBody BenchL2(benchL2);

		BenchL2.RotateModelMatrix(glm::vec3(0.0, 1.0, 0.0), 90.0f);
		BenchL2.TranslateModelMatrix(position + glm::vec3(0.0, -0.1, 0.0));
		//houseM.Translate(position);
		BenchL2.Scale(scale);

		StaticObjects.push_front(BenchL2);
		Model bench2("assets/Obj/StaticObjects/bench1.obj");

		StaticModelList.push_front(bench2);
	}

	void Stone1(glm::vec3 position, glm::vec3 scale) {
		//Model stone1("assets/Obj/DynamicObjects/RockSet05-OBJ/RockSet05A.obj");
		//Model stone1("assets/Obj/DynamicObjects/Cone/Cone.fbx");
		Model stone1("assets/Obj/DynamicObjects/Sarah/RockA.obj");
		ObjectModelList.push_front(stone1);

		Hitbox Stone1(mPhysics, 0.7f, 2.9f, 0.7f, position, SCR_WIDTH, SCR_HEIGHT);
		Stone1.scale(scale);
		Stone1.finalScale *= glm::vec3(0.275f, 0.275f, 0.275f);
		Stone1.finalTranslate = glm::vec3(0.0, 0.0, 0.0);

		Stone1.rotate(glm::vec3(0.0, 0.0, 1.0), 90.0f);
		BuildingObjects.push_front(Stone1);
	}

	void Stone2(glm::vec3 position, glm::vec3 scale) {
		Model stone2("assets/Obj/DynamicObjects/Sarah/RockB.obj");
		ObjectModelList.push_front(stone2);

		Hitbox Stone2(mPhysics, 1.4f, 1.9f, 1.4f, position, SCR_WIDTH, SCR_HEIGHT);
		Stone2.scale(scale);
		Stone2.finalScale *= glm::vec3(0.275f, 0.275f, 0.275f);
		Stone2.finalTranslate = glm::vec3(0.0, -0.0, 0.0);
		BuildingObjects.push_front(Stone2);
	}

	void Stone3(glm::vec3 position, glm::vec3 scale) {
		Model stone3("assets/Obj/DynamicObjects/Sarah/RockC.obj");
		ObjectModelList.push_front(stone3);

		Hitbox Stone3(mPhysics, 1.0f, 2.1f, 1.0f, position, SCR_WIDTH, SCR_HEIGHT);
		Stone3.scale(scale);
		Stone3.finalScale *= glm::vec3(0.275f, 0.275f, 0.275f);
		Stone3.finalTranslate = glm::vec3(0.0, 0.0, 0.0);
		BuildingObjects.push_front(Stone3);
	}

	void Crate1(glm::vec3 position, glm::vec3 scale) {
		Model crate1("assets/Obj/DynamicObjects/Sarah/crate.obj");
		ObjectModelList.push_front(crate1);

		Hitbox Crate1(mPhysics, 0.5f, 0.2f, 0.23f, position, SCR_WIDTH, SCR_HEIGHT);
		Crate1.scale(scale);
		Crate1.finalScale *= glm::vec3(1.0f, 1.0f, 1.0f);
		Crate1.finalTranslate = glm::vec3(0.0, -0.2, -0.2);
		BuildingObjects.push_front(Crate1);
	}

	void Ladder1(glm::vec3 position, glm::vec3 scale) {
		//set Hitboxes 1st vec3 = size of HB, 2nd vec3 = position of HB
		Model ladder1("assets/Obj/DynamicObjects/Sarah/ladder.obj");
		ObjectModelList.push_front(ladder1);

		Hitbox Ladder1(mPhysics, 4.5f, 0.2f, 1.4f, position, SCR_WIDTH, SCR_HEIGHT);
		Ladder1.scale(scale);
		Ladder1.finalScale *= glm::vec3(1.3f, 1.6f, 1.6f);
		Ladder1.finalTranslate = glm::vec3(0.0, -0.1, -0.0);
		BuildingObjects.push_front(Ladder1);
	}

	void Lamp(glm::vec3 position, glm::vec3 scale) {
		Model LampBase = ("assets/Obj/StaticObjects/Light_Base.obj");
		Model LampBalls = ("assets/Obj/StaticObjects/Light_ball_middle.obj");
		StaticHitbox LampLSide(mPhysics, 0.2f, 8.5f, 0.2f, glm::vec3(0.0f, 7.5f, 12.0f));
		StaticHitbox LampRSide(mPhysics, 0.2f, 8.5f, 0.2f, glm::vec3(0.0f, 7.5f, -12.0f));

		//StaticHitbox Lamp1(mPhysics, 0.4f, 0.4f, 0.4f, glm::vec3(0.0f, 8.0f, 8.0f), false);
		//Lamp1.addModel(LampBalls);
		//StaticHitbox Lamp2(mPhysics, 0.4f, 0.4f, 0.4f, glm::vec3(0.0f, 8.0f, 2.5f), false);
		//Lamp2.addModel(LampBalls);
		//StaticHitbox Lamp3(mPhysics, 0.4f, 0.4f, 0.4f, glm::vec3(0.0f, 8.0f, -2.5f), false);
		//Lamp3.addModel(LampBalls);
		//StaticHitbox Lamp4(mPhysics, 0.4f, 0.4f, 0.4f, glm::vec3(0.0f, 8.0f, -8.0f), false);
		//Lamp4.addModel(LampBalls);

		StaticHitbox Lamps(mPhysics, 0.2f, 0.2f, 12.0f, glm::vec3(0.0f, 16.0f, 0.0f), false);
		Lamps.addModel(LampBalls);
		Lamps.TranslateModelMatrix(glm::vec3(0.0, -16.75f, -5.0));

		KomplexStaticBody lamp(LampLSide);
		lamp.addStaticBody(LampRSide);
		//lamp.addStaticBody(Lamp1);
		//lamp.addStaticBody(Lamp2);
		//lamp.addStaticBody(Lamp3);
		//lamp.addStaticBody(Lamp4);
		lamp.addStaticBody(Lamps);

		lamp.Scale(scale);
		lamp.Translate(position);

		lamp.TranslateModelMatrix(glm::vec3(0.0, -0.75f, -5.0));
		StaticObjects.push_front(lamp);

		StaticModelList.push_front(LampBase);

		Light LampLight;
		LampLight.setPointLight(glm::vec3(0.0, 12.0, -2.5), glm::vec3(1.0, 1.0, 1.0));
		//
		numberOfPointLights++;
		pointLight.push_front(LampLight);
	}
	void LampL(glm::vec3 position, glm::vec3 scale) {
		Model LampBallsL = ("assets/Obj/StaticObjects/Light_balls_L.obj");
		StaticHitbox Lamps(mPhysics, 0.2f, 0.2f, 12.0f, glm::vec3(0.0f, 16.0f, 3.0f), false);
		Lamps.addModel(LampBallsL);

		Light LampLight;
		LampLight.setPointLight(glm::vec3(0.0, 12.0, 0.0), glm::vec3(1.0, 2.0, 1.0));
		
		numberOfPointLights++;
		pointLight.push_front(LampLight);

	}
};