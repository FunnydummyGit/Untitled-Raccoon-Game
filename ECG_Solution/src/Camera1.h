#pragma once
#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
	enum Camera_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
	};

	glm::vec3 position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	// euler Angles
	float Yaw;
	float Pitch;

	float fov;
	float width;
	float height;
	float cameraNear;
	float cameraFar;
	glm::mat4 view;
	glm::mat4 projection;


	// Constructor
	Camera(glm::vec3 Position, float Fov, float Width, float Height, float CameraNear, float CameraFar, float yaw, float pitch) {
		position = Position;
		fov = Fov;
		width = Width;
		height = Height;
		cameraNear = CameraNear;
		cameraFar = CameraFar;

		Yaw = yaw;
		Pitch = pitch;

		view = glm::translate(view, position);
		view = glm::inverse(view);
		projection = glm::perspective((float)glm::radians(fov), width / height, cameraNear, cameraFar);

		Front = glm::normalize(-position);
		Up = glm::vec3(0.0f, 1.0f, 0.0f);
		WorldUp = Up;
		Right = glm::normalize(glm::cross(Front, WorldUp));
	}


	glm::mat4 getViewMatrix() {
		//glm::mat4 viewWorld = view * world;
		//return viewWorld;
		return view;
	}


	glm::vec3 getPosition() {
		return position;
	}

	void setPosition(glm::vec3 pos) {
		position = pos;

		view = glm::translate(glm::mat4(1), position);
		view = glm::inverse(view);

		Yaw = -90.0f;
		Pitch = 0.0f;

		Front = glm::normalize(-position);
		Up = glm::vec3(0.0f, 1.0f, 0.0f);
		WorldUp = Up;
		Right = glm::normalize(glm::cross(Front, WorldUp));
	}

	void updateCamera(double x, double y, double lastX, double lastY, bool dragging, GLboolean constrainPitch = true) {

		if (dragging) {
			//Calculate the distance the mouse was moved, between the last and the current frame
			double disX = x - lastX;
			double disY = lastY - y;

			float scaleX = disX / width;
			float scaleY = disY / height;
			float rotSpeed = 350.0f;

			Yaw += scaleX * rotSpeed;
			Pitch += scaleY * rotSpeed;

			// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (constrainPitch)
			{
				if (Pitch > 89.0f)
					Pitch = 89.0f;
				if (Pitch < -89.0f)
					Pitch = -89.0f;
			}

			updateCameraVectors();
		}

		if (position.y < 0) {
			position.y = 0;
		}

		view = calculate_lookAt_matrix(position, position + Front, Up);

		//ZOOM

		//fov = (float)zoom;
		//if (fov < 1.0f)
		//	fov = 1.0f;
		//if (fov > 90.0f)
		//	fov = 90.0f;

		projection = glm::perspective((float)glm::radians(fov), width / height, cameraNear, cameraFar);

		//new Position
		glm::mat4 invView = glm::inverse(view);
		position.x = invView[3][0];
		position.y = invView[3][1];
		position.z = invView[3][2];
	}

	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		glm::vec3 nPosition = position;

		float velocity = 15.0f * deltaTime;
		if (direction == FORWARD)
			nPosition += Front * velocity;
		if (direction == BACKWARD)
			nPosition -= Front * velocity;
		if (direction == LEFT)
			nPosition -= Right * velocity;
		if (direction == RIGHT)
			nPosition += Right * velocity;

		//view = glm::translate(view, position - nPosition);
		position = nPosition;
	}

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}

	// Custom implementation of the LookAt function
	glm::mat4 calculate_lookAt_matrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp)
	{
		// 1. Position = known
		// 2. Calculate cameraDirection
		glm::vec3 zaxis = glm::normalize(position - target);
		// 3. Get positive right axis vector
		glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
		// 4. Calculate camera up vector
		glm::vec3 yaxis = glm::cross(zaxis, xaxis);

		// Create translation and rotation matrix
		// In glm we access elements as mat[col][row] due to column-major layout
		glm::mat4 translation; // Identity matrix by default
		translation[3][0] = -position.x; // Third column, first row
		translation[3][1] = -position.y;
		translation[3][2] = -position.z;
		glm::mat4 rotation;
		rotation[0][0] = xaxis.x; // First column, first row
		rotation[1][0] = xaxis.y;
		rotation[2][0] = xaxis.z;
		rotation[0][1] = yaxis.x; // First column, second row
		rotation[1][1] = yaxis.y;
		rotation[2][1] = yaxis.z;
		rotation[0][2] = zaxis.x; // First column, third row
		rotation[1][2] = zaxis.y;
		rotation[2][2] = zaxis.z;

		// Return lookAt matrix as combination of translation and rotation matrix
		return rotation * translation; // Remember to read from right to left (first translation then rotation)
	}
};
