#pragma once
#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/model.h>

class StaticHitbox {
private:
physx::PxShape* shape;
float width;
float height;
float depth;
bool shadow;

glm::vec3 scaleVector = glm::vec3(1);
glm::vec3 finalTranslation = glm::vec3(0);

glm::mat4 modelMatrix = glm::mat4(1);

Model model = ("assets/Obj/StaticObjects/Light_Balls.obj");

public:
	physx::PxMaterial* mMaterial = NULL;
	physx::PxPhysics* mPhysics = NULL;
	physx::PxRigidStatic* body;
	physx::PxTransform t;
	
	glm::vec3 position;

	StaticHitbox(physx::PxPhysics* Physics, float x, float y, float z, glm::vec3 Position, bool shadow = true) {
		mPhysics = Physics;
		mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f); // Kann vlt noch übergeben werden
		width = x;
		height = y;
		depth = z;

		this->shadow = shadow;

		position = Position;

		modelMatrix = glm::translate(modelMatrix, position);

		shape = mPhysics->createShape(physx::PxBoxGeometry(width, height, depth), *mMaterial);
		
		t = physx::PxTransform(physx::PxVec3(position.x, position.y, position.z));
		body = mPhysics->createRigidStatic(t);
		body->attachShape(*shape);
	}

	void Translate(glm::vec3 p) {
		position += p;
		t = body->getGlobalPose();
		t = physx::PxTransform(physx::PxVec3(t.p.x + p.x, t.p.y + p.y, t.p.z + p.z));
		body->setGlobalPose(t);
		t = body->getGlobalPose();
		modelMatrix = glm::translate(modelMatrix, p);
	}

	void Scale(glm::vec3 p) {
		body->detachShape(*shape);
		width *= p.x;
		height *= p.y;
		depth *= p.z;

		shape->setGeometry(physx::PxBoxGeometry(width, height, depth));
		body->attachShape(*shape);

		scaleVector += p;
	}

	void addModel(Model model) {
		if (!shadow) {
			this->model = model;
		}
	}

	void drawModel(Shader* shader) {
		if (!shadow)
		{
			shader->setBool("shadowBool", false);
			shader->setMat4("model", getModelMatrix());
			model.Draw(*shader);
			shader->setBool("shadowBool", true);
		}
	}

	bool getShadow() {
		return shadow;
	}

	void TranslateModelMatrix(glm::vec3 p) {
		modelMatrix = glm::translate(modelMatrix, p);
	}
	
	glm::mat4 getModelMatrix() {
		return glm::scale(glm::translate(modelMatrix, finalTranslation * scaleVector), scaleVector);
	}
};