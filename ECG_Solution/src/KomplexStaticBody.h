#pragma once
#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include "StaticHitbox.h"
#include "LinkedList.h"
#include <list>


class KomplexStaticBody
{
private:
	glm::mat4 modelMatrix = glm::mat4(1);
	glm::vec3 finalTranslation;
	glm::vec3 rotationVector = glm::vec3( 0.0, 1.0, 0.0);
	float degree;
	glm::vec3 scaleVector;
public:

	list <StaticHitbox> bodies;
	glm::vec3 position = glm::vec3(0);


	KomplexStaticBody(StaticHitbox Hitbox){
		bodies.push_front(Hitbox);
	}

	void addStaticBody(StaticHitbox Hitbox) {
		bodies.push_front(Hitbox);
	}

	void Translate(glm::vec3 p) {
		if (bodies.size() > 1) {
			list <StaticHitbox> next = bodies;
			while (next.size() > 0)
			{
				glm::vec3 fromMiddle = next.front().position - position;
				glm::vec3 translatedMiddle = fromMiddle + p - next.front().position;
				next.front().Translate(translatedMiddle);
				next.pop_front();
			}
		}
		else {
			bodies.front().Translate(p + position);
			//next.front().Translate(glm::vec3(-p.x, -p.y, -p.z));
		}

		modelMatrix = glm::translate(modelMatrix, p);
		position += p;
	}

	void Scale(glm::vec3 p) {
		list <StaticHitbox> next = bodies;
		while (next.size() > 0)
		{
			next.front().Scale(p);
			glm::vec3 fromMiddle = next.front().position - position;
			glm::vec3 scaledMiddle = fromMiddle * p - next.front().position;
			next.front().Translate(scaledMiddle);
			next.pop_front();
		}

		scaleVector += p;
	}

	void addActor(physx::PxScene* scene) {
		list <StaticHitbox> next = bodies;
		while (next.size() > 0)
		{
			scene->addActor(*next.front().body);
			next.pop_front();
		} 
	}

	void TranslateModelMatrix(glm::vec3 p) {
		 finalTranslation = p;
	}

	void RotateModelMatrix(glm::vec3 p, float d) {
		rotationVector = p;
		degree = d;
	}

	glm::mat4 getModelMatrix() {
		return glm::scale(glm::rotate(glm::translate(modelMatrix, finalTranslation * scaleVector), glm::radians(degree), rotationVector), scaleVector);
	}

	bool getShadow() {
		list <StaticHitbox> next = bodies;
		bool shadow = true;
		while (next.size() > 0)
		{
			if (!next.front().getShadow()) {
				shadow = false;
			}
			next.pop_front();
		}
		return shadow;
	}

	void drawModel(Shader* shader){
		list <StaticHitbox> next = bodies;
		while (next.size() > 0)
		{
			if (!next.front().getShadow()) {
				next.front().drawModel(shader);
			}
			next.pop_front();
		}
	}
};
