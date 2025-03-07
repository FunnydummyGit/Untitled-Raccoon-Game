#pragma once
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "PlayerController.h"

class Camera2 {
public:
	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods

	glm::vec3 position;

	float fov;
	float width;
	float height;
	float cameraNear;
	float cameraFar;
	float Yaw;
	glm::mat4 translationMatrix;
	glm::mat4 rotationMatrix;
	glm::mat4 firstRotation;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 world = glm::mat4(1.0f);


	// Constructor
	Camera2(float Fov, float Width, float Height, float CameraNear, float CameraFar, float yaw) {
		fov = Fov;
		width = Width;
		height = Height;
		cameraNear = CameraNear;
		cameraFar = CameraFar;
		Yaw = yaw;

		projection = glm::perspective((float)glm::radians(fov), width / height, cameraNear, cameraFar);
		translationMatrix = glm::translate(translationMatrix, glm::vec3(0.0, -6.0, 0.0));
		rotationMatrix = glm::rotate(rotationMatrix, (float)glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
		firstRotation = glm::rotate(glm::mat4(1), (float)glm::radians(20.0f), glm::vec3(1.0, 0.0, 0.0));

		view = glm::inverse(translationMatrix * rotationMatrix);
	}

	glm::mat4 getViewMatrix() {
		glm::mat4 viewWorld = view * world;
		return viewWorld;
	}


	glm::vec3 getPosition() {
		return position;
	}

	void updateCamera(PlayerController* model) {
		glm::vec3 nPosition = model->position - model->Front * glm::vec3(12, 0, 12);
		translationMatrix = glm::translate(translationMatrix, position - nPosition);
		position = nPosition;

		float nYaw = Yaw - model->Yaw;
		rotationMatrix = glm::rotate(rotationMatrix, (float) glm::radians(nYaw), glm::vec3(0.0, 1.0, 0.0));
		Yaw = model->Yaw;


		glm::mat4 inv = glm::mat4(1);
		//inv = glm::inverse(inv);
		view = inv;
		view *= firstRotation;
		view *= glm::inverse(rotationMatrix);
		view *= translationMatrix;
		//view = rotationMatrix * translationMatrix  * inv;

		projection = glm::perspective((float)glm::radians(fov), width / height, cameraNear, cameraFar);
	}
};
