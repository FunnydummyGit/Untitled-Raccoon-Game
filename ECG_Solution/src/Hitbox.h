#pragma once
#include <PxPhysicsAPI.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Hitbox {
public:
	physx::PxMaterial* mMaterial = NULL;
	physx::PxPhysics* mPhysics = NULL;
	float width;
	float height;
	float depth;
	float radius;
	physx::PxTransform t;
	physx::PxRigidDynamic* body;
	glm::mat4 modelMatrix = glm::mat4(1);
	glm::vec3 position;
	physx::PxShape* shape;
	float ScreenW;
	float ScreenH;
	glm::vec3 finalScale;
	glm::vec3 finalTranslate;

	Hitbox(physx::PxPhysics *Physics, float x, float y, float z, glm::vec3 Pos, float sw, float sh) {
		mPhysics = Physics;
		mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f); // Kann vlt noch übergeben werden
		width = x;
		height = y;
		depth = z;
		position = Pos;

		ScreenW = sw;
		ScreenH = sh;

		float halfExtent = .5f;
		shape = mPhysics->createShape(physx::PxBoxGeometry(width, height, depth), *mMaterial);

		t = physx::PxTransform(physx::PxVec3(position.x, position.y, position.z));
		body = mPhysics->createRigidDynamic(t.transform(t));
		body->attachShape(*shape);
		physx::PxRigidBodyExt::updateMassAndInertia(*body, 500.0f);



		modelMatrix = glm::translate(modelMatrix, position);

		radius = (width + height + depth) / 3;
	}

	physx::PxRigidDynamic* GetBody() {
		return body;
	}

	void translate(glm::vec3 trans) {
		t = physx::PxTransform(physx::PxVec3(trans.x + position.x, trans.y + position.y, trans.z + position.z));
		position += trans;
		body->setGlobalPose(t);
	}

	void rotate(glm::vec3 axis, float degree) {
		t = body->getGlobalPose();
		t.rotateInv(PxVec3(axis.x * degree, axis.y * degree, axis.z * degree));
		body->setGlobalPose(t);
	}

	void scale(glm::vec3 vec) {
		body->detachShape(*shape);
		finalScale += vec;
		shape = mPhysics->createShape(physx::PxBoxGeometry(width * vec.x, height * vec.y, depth * vec.z), *mMaterial);
		body->attachShape(*shape);


		width = width* vec.x; 
		height = height* vec.y; 
		depth = depth* vec.z;
		radius = (width + height + depth) / 3;
	}


	void rotateModelMatrix(float degree, glm::vec3 vec) {
		modelMatrix = glm::rotate(modelMatrix, (float)glm::radians(degree), vec);
	}

	void updateModelMatrix() {
		physx::PxMat44 pose = body->getGlobalPose();

		for (size_t i = 0; i < 4; i++)
		{
			for (size_t j = 0; j < 4; j++) {
				modelMatrix[i][j] = pose[i][j];
			}

		}
		position.x = modelMatrix[3][0];
		position.y = modelMatrix[3][1];
		position.z = modelMatrix[3][2];
	}

	void mouseInput(double x, double y, double lastX, double lastY, float scroll_offset, Camera camera) {
		double diffX = x - lastX;
		double diffY = lastY - y;

		float rightX = -sin((float)glm::radians(camera.Yaw));
		//float rightY = 0;
		float rightZ = cos((float)glm::radians(camera.Yaw));

		float upX = cos((float)glm::radians(-camera.Pitch + 90.0f)) * -cos((float)glm::radians(-camera.Yaw));
		float upY = sin((float)glm::radians(-camera.Pitch + 90.0f));
		float upZ = cos((float)glm::radians(-camera.Pitch + 90.0f)) * sin((float)glm::radians(-camera.Yaw));

		float frontX = cos((float)glm::radians(camera.Yaw)) * sin((float) glm::radians(-camera.Pitch + 90));
		float frontY = cos((float)glm::radians(-camera.Pitch + 90.0f));
		float frontZ = sin((float)glm::radians(camera.Yaw)) * sin((float) glm::radians(-camera.Pitch + 90));


		float cameraDistance = glm::length(camera.position - position) * (camera.fov / 60.0f);

		float scaleX = diffX / ScreenW * cameraDistance * 2; //* distance from Camera;
		float scaleY = diffY / ScreenH * cameraDistance; //* distance from Camera;

		float ydirection = upY * scaleY + frontY * scroll_offset * 1.5f;
		if (position.y - height / 2 < 0 && ydirection < 0) {
			ydirection = 0;
		}
		
		glm::vec3 vec = glm::vec3 (
			rightX * scaleX + upX * scaleY + frontX * scroll_offset,
			ydirection,
			rightZ * scaleX + upZ * scaleY + frontZ * scroll_offset);

		body->clearForce();
		translate(vec);
	}

	glm::mat4 getModelMatrix() {
		glm::mat4 finalMatrix = glm::translate(modelMatrix, finalTranslate * finalScale);
		return glm::scale(finalMatrix, finalScale);
	}
};