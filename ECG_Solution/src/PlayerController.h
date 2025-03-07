#pragma once
#include <PxPhysicsAPI.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace physx;

class PlayerController
{
private:
	float notfallingframes = 0.0f;
	PxVec3 disp;
	bool runJump = false;
public:
	enum Player_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN,
	};


	float Yaw = 90.0f;
	//float modelYaw;

	bool jumping = false;
	float jumpVelocity = 0.0f;
	float gravity = -20.0f;

	PxController* c;
	PxControllerManager* manager;

	glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Front;
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Right;
	glm::mat4 modelMatrix = glm::mat4(1);

	glm::vec3 movement = glm::vec3(0);
	glm::vec3 jumpMovement = glm::vec3(0);



	PlayerController();
	~PlayerController();

	void createPlayerController(PxPhysics *physics ,PxControllerManager* Manager , float width, float height, float depth, glm::vec3 position, float yaw) {

		PxMaterial* material = physics->createMaterial(0.5f, 0.5f, 0.6f);

		Yaw = yaw;
		updateMovementVectors();

		manager = Manager;

		PxBoxControllerDesc desc;
		desc.upDirection = PxVec3(Front.x, Front.y, Front.z);
		desc.halfForwardExtent = width;
		desc.halfHeight = depth;
		desc.halfSideExtent = height;
		desc.material = material;
		desc.position = PxExtendedVec3(position.x, position.y, position.z);
		bool valid = desc.isValid();

		c = manager->createController(desc);

		this->position = position;
		//c->setPosition(PxExtendedVec3(position.x, position.y, position.z));
		modelMatrix = glm::mat4(1);

		physx::PxMat44 pose = c->getActor()->getGlobalPose();

		for (size_t j = 0; j < 4; j++) {
			modelMatrix[3][j] = pose[3][j];
		}
	}

	void move(Player_Movement direction, bool _running) {
		if (direction == FORWARD)
			movement += Front;
		if (direction == BACKWARD)
			movement -= Front;
		if (direction == LEFT)
			movement -= Right;
		if (direction == RIGHT)
			movement += Right;
		
		if (!jumping && direction == UP) {
			jumpVelocity = 14.0f;
			gravity = -20.0f;
			jumping = true;
			if (_running) {
				runJump = true;
			}
		}
	}

	glm::vec3 jump(float deltaTime) {

		float jumpY = jumpVelocity * deltaTime;
		jumpVelocity += gravity * deltaTime;
		gravity -= 20.0f * deltaTime;

		return glm::vec3(0.0, jumpY, 0.0);
	}

	void rotate(double x, double y, double lastX, double lastY, float width, float height) {

		//Calculate the distance the mouse was moved, between the last and the current frame
		double disX = lastX - x;
		double disY = lastY - y;

		//Tweak these values to change the sensetivity
		float scaleX = abs(disX) / width;
		float scaleY = abs(disY) / height;
		float rotSpeed = 350.0f;

		//Horizontal rotation 
		if (disX < 0) {
			Yaw += rotSpeed * scaleX;
		}
		else if (disX > 0) {
			Yaw -= rotSpeed * scaleX;
		}
		updateMovementVectors();
		c->setUpDirection(PxVec3(Front.x, Front.y, Front.z));
	}

	void updatePlayerCharacter(float deltaTime, bool _running) {
		glm::vec3 nPosition = position;

		float runfactor = 1.0f;
		if ((_running && !jumping) || runJump) {
			runfactor = 1.75f;
		}

		if (movement != glm::vec3(0))
		movement = glm::normalize(movement);

		jumpMovement = jump(deltaTime);

		float velocity = 10.0f * deltaTime * runfactor;
		nPosition += movement * velocity + jumpMovement;

		glm::vec3 gDisp = nPosition - position;

		PxVec3 disp = PxVec3(gDisp.x, gDisp.y, gDisp.z);

		PxControllerCollisionFlags collisionFlags =
			c->move(disp, 0.0f, deltaTime, NULL, NULL);
		PxExtendedVec3 pos = c->getPosition();
		nPosition = glm::vec3(pos.x, pos.y, pos.z);

		physx::PxMat44 pose = c->getActor()->getGlobalPose();

		for (size_t j = 0; j < 4; j++) {
			modelMatrix[3][j] = pose[3][j];
		}

		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -1.9, 0.0)); //kann vlt noch das Model angepasst werden


		if (nPosition.y - position.y < 0.005 && nPosition.y - position.y > -0.005) {
			notfallingframes += deltaTime;
		}
		else {
			notfallingframes = 0.0f;
		}

		if (notfallingframes > 0.5f && jumpVelocity < -10) {
			jumping = false;
			runJump = false;
			jumpVelocity = -9.81f;
		}
		position = nPosition;
		movement = glm::vec3(0);
		jumpMovement = glm::vec3(0);
	}

	glm::mat4 GetFinalModelMatrix() {
		glm::mat4 finalMatrix;
		finalMatrix = glm::rotate(modelMatrix, (float)glm::radians(-Yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		finalMatrix = glm::rotate(finalMatrix, (float)glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//finalMatrix = glm::rotate(finalMatrix, (float)glm::radians(-180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//finalMatrix = glm::translate(finalMatrix, glm::vec3(0.0, 0.5, 0.0));
		return finalMatrix;
	}


	void setPosition(glm::vec3 position, float yaw) {
		Yaw = yaw;
		this->position = position;
		c->setPosition(PxExtendedVec3(position.x, position.y, position.z));

		Front = glm::vec3(0.0f, 0.0f, 1.0f);
		Up = glm::vec3(0.0f, 1.0f, 0.0f);
		Right = glm::normalize(glm::cross(Front, Up));
	}

	void release() {
		c->release();
	}


private:

	void updateMovementVectors() {
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw));
		front.y = 0;
		front.z = sin(glm::radians(Yaw));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, Up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};

PlayerController::PlayerController()
{
}

PlayerController::~PlayerController()
{
}
