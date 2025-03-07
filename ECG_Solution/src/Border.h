#pragma once
#include "PxPhysicsAPI.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/model_animation.h>
#include <list>;

class Border {
public:
	physx::PxMaterial* mMaterial = NULL;
	physx::PxPhysics* mPhysics = NULL;
	physx::PxRigidStatic* plane1;
	physx::PxRigidStatic* plane2;
	physx::PxRigidStatic* plane3;
	physx::PxRigidStatic* plane4;
	float width;
	float depth;
	float length;
	float scale;

	Border();
	~Border();

	void setBorder(physx::PxPhysics* Physics, float x, float z, float modelLength, float modelScale) {
		mPhysics = Physics;
		mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f); // Kann vlt noch übergeben werden

		length = modelLength * modelScale;
		scale = modelScale;
		width = x * length;
		depth = z * length;

		plane1 = PxCreatePlane(*mPhysics, physx::PxPlane(-1, 0, 0, width / 2), *mMaterial);
		plane2 = PxCreatePlane(*mPhysics, physx::PxPlane(1, 0, 0, width / 2), *mMaterial);
		plane3 = PxCreatePlane(*mPhysics, physx::PxPlane(0, 0, -1, depth / 2), *mMaterial);
		plane4 = PxCreatePlane(*mPhysics, physx::PxPlane(0, 0, 1, depth / 2), *mMaterial);
	}

	void Draw(Shader shader, Model* model) {
		if (length > 0) {
			glm::mat4 modelMatrix ;

			//Plane1
			modelMatrix = glm::translate(glm::mat4(1), glm::vec3((-width / 2) + (length / 2), -1.0f, depth / 2));
			for (size_t i = 0; i < width / length; i++)
			{
				glm::mat4 scaled = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));
				shader.setMat4("model", scaled);
				model->Draw(shader);

				modelMatrix = glm::translate(modelMatrix, glm::vec3(length, 0.0f, 0.0f));
			}

			//Plane 2
			
			modelMatrix = glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::translate(modelMatrix, glm::vec3((-width / 2) + (length / 2), -1.0f, depth / 2));

			for (size_t i = 0; i < (width / length); i++)
			{
				glm::mat4 scaled = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));
				shader.setMat4("model", scaled);
				model->Draw(shader);

				modelMatrix = glm::translate(modelMatrix, glm::vec3(length, 0.0f, 0.0f));
			}

			//Plane 3
			modelMatrix = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::translate(modelMatrix, glm::vec3((-depth / 2) + (length / 2), -1.0f, width / 2));

			for (size_t i = 0; i < depth / length; i++)
			{
				glm::mat4 scaled = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));
				shader.setMat4("model", scaled);
				model->Draw(shader);

				modelMatrix = glm::translate(modelMatrix, glm::vec3(length, 0.0f, 0.0f));
			}

			modelMatrix = glm::rotate(glm::mat4(1), glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::translate(modelMatrix, glm::vec3((-depth / 2) + (length / 2), -1.0f, width / 2));

			for (size_t i = 0; i < depth / length; i++)
			{
				glm::mat4 scaled = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));
				shader.setMat4("model", scaled);
				model->Draw(shader);

				modelMatrix = glm::translate(modelMatrix, glm::vec3(length, 0.0f, 0.0f));
			}
		}
	}
};

Border::Border()
{
}

Border::~Border()
{
}
