#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Goal
{
public:
	glm::vec3 position;
	glm::mat4 modelMatrix;

	float radius = 0;


	Goal();
	~Goal();

	void initGoal(glm::vec3 pos, float r) {
		position = pos;
		modelMatrix = glm::translate(glm::mat4(1), pos);

		radius = r;
	}

	bool checkIfReached(glm::vec3 playerPosition) {
		float x = pow((playerPosition.x - position.x), 2);
		float y = pow((playerPosition.y - position.y), 2);
		float z = pow((playerPosition.z - position.z), 2);

		bool won = x + y + z <= pow(radius, 2);
		if (won) {
			won = won;
		}

		return won;
	}
};

Goal::Goal()
{
}

Goal::~Goal()
{
}
