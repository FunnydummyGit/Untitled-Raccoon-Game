#include <learnopengl/model_animation.h>
#include <learnopengl/shader_m.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <list>
#include <iostream>
#include <stdlib.h>


class World
{
private:
	Model ground = "assets/Obj/World/ground_new.obj";
	Model flower = "assets/Obj/World/Flowers.obj";
	Model tree1 = "assets/Obj/World/tree1.obj";
	Model tree2 = "assets/Obj/World/tree2.obj";
	Model tree3 = "assets/Obj/World/tree3.obj";
	Model bush1 = "assets/Obj/World/bush3.obj";

public:

	list <glm::vec3> tree1Positions;
	list <glm::vec3> tree1Scale;
	list <glm::vec3> tree2Positions;
	list <glm::vec3> tree2Scale;
	list <glm::vec3> tree3Positions;
	list <glm::vec3> tree3Scale;

	list <glm::vec3> flowerPositions;

	list <glm::vec3> bush1Positions;
	list <glm::vec3> bush1Scale;

	glm::mat4 modelMatrix = glm::mat4(1);
	glm::vec3 translate;
	glm::vec3 scale;

	World();
	~World();

	void generate(glm::mat4 Matrix, glm::vec3 translate, glm::vec3 scale) {

		modelMatrix = Matrix;
		this->scale = scale;
		this->translate = translate;

		glm::vec3 tall = glm::vec3(1.5, 1.5, 1.5);
		glm::vec3 medium = glm::vec3(1.35, 1.35, 1.35);
		glm::vec3 small = glm::vec3(1.2, 1.2, 1.2);


		//left trees
		tree1Positions.push_front(glm::vec3(-10.5, 0.0, 4.8));
		tree1Scale.push_front(tall);
		tree1Positions.push_front(glm::vec3(-7.5, 0.0, 9.0));
		tree1Scale.push_front(small);
		tree1Positions.push_front(glm::vec3(5.0, 0.0, 10.5));
		tree1Scale.push_front(tall);
		//right trees
		tree1Positions.push_front(glm::vec3(-10.0, 0.0, -6.0));
		tree1Scale.push_front(small);
		tree1Positions.push_front(glm::vec3(-2.5, 0.0, -10.5));
		tree1Scale.push_front(small);
		tree1Positions.push_front(glm::vec3(0.0, 0.0, -11.5));
		tree1Scale.push_front(medium);
		tree1Positions.push_front(glm::vec3(7.5, 0.0, -9.0));
		tree1Scale.push_front(tall);


		tree2Positions.push_front(glm::vec3(-5.5, 0.0, 8.5));
		tree2Scale.push_front(small);
		tree2Positions.push_front(glm::vec3(7.5, 0.0, 10.0));
		tree2Scale.push_front(medium);
		tree2Positions.push_front(glm::vec3(8.5, 0.0, 9.5));
		tree2Scale.push_front(small);

		tree2Positions.push_front(glm::vec3(-8.0, 0.0, -7.3));
		tree2Scale.push_front(medium);
		tree2Positions.push_front(glm::vec3(-6.5, 0.0, -8.0));
		tree2Scale.push_front(tall);
		tree2Positions.push_front(glm::vec3(-10.0, 0.0, 0.5));
		tree2Scale.push_front(small);


		tree3Positions.push_front(glm::vec3(-2.5, 0.0, 11.0));
		tree3Scale.push_front(small);
		tree3Positions.push_front(glm::vec3(-6.0, 0.0, -11.0));
		tree3Scale.push_front(small);
		tree3Positions.push_front(glm::vec3(-11.0, 0.0, -2.5));
		tree3Scale.push_front(medium);

		tree3Positions.push_front(glm::vec3(6.0, 0.0, -10.5));
		tree3Scale.push_front(medium);


		flowerPositions.push_front(glm::vec3(-0.85, 0.0, 0.15));
		flowerPositions.push_front(glm::vec3(-1.1, 0.0, 1.5));
		flowerPositions.push_front(glm::vec3(-1.8, 0.0, 0.15));

		
		flowerPositions.push_front(glm::vec3(-0.0, 0.0, 12.0));
		flowerPositions.push_front(glm::vec3(0.5, 0.0, 13.0));
		flowerPositions.push_front(glm::vec3(1.0, 0.0, 12.2));

		flowerPositions.push_front(glm::vec3(-7.0, 0.0, 15.2));
		flowerPositions.push_front(glm::vec3(-8.0, 0.0, 15.8));
		flowerPositions.push_front(glm::vec3(-8.6, 0.0, 15.8));

		flowerPositions.push_front(glm::vec3(-12.6, 0.0, 6.5));
		flowerPositions.push_front(glm::vec3(-12.6, 0.0, 5.5));
		flowerPositions.push_front(glm::vec3(-17.6, 0.0, -1.6));
		flowerPositions.push_front(glm::vec3(-18.0, 0.0, -1.0));

		bush1Positions.push_front(glm::vec3(3.0, 0.35, 12.0));
		bush1Scale.push_front(glm::vec3(1.0, 1.0, 1.0));
		bush1Positions.push_front(glm::vec3(10.0, 0.35, -6.8));
		bush1Scale.push_front(medium);
		bush1Positions.push_front(glm::vec3(-10.0, 0.35, -0.4));
		bush1Scale.push_front(glm::vec3(1.0, 1.0, 1.0));
		bush1Positions.push_front(glm::vec3(-10.5, 0.35, -1.0)); 
		bush1Scale.push_front(tall);
	}
	void Draw(Shader* shader, bool shadow) {

		shader->setMat4("model", modelMatrix);
		ground.Draw(*shader);

		if (!shadow) {
			list <glm::vec3> positions = tree1Positions;
			list <glm::vec3> scales = tree1Scale;
			for (size_t i = 0; i < tree1Positions.size(); i++)
			{
				glm::mat4 matrix = glm::translate(modelMatrix, positions.front());
				matrix = glm::scale(matrix, scales.front());
				shader->setMat4("model", matrix);
				tree1.Draw(*shader);
				positions.pop_front();
				scales.pop_front();
			}

			positions = tree2Positions;
			scales = tree2Scale;
			for (size_t i = 0; i < tree2Positions.size(); i++)
			{
				glm::mat4 matrix = glm::translate(modelMatrix, positions.front());
				matrix = glm::scale(matrix, scales.front());
				shader->setMat4("model", matrix);
				tree2.Draw(*shader);
				positions.pop_front();
				scales.pop_front();
			}

			positions = tree3Positions;
			scales = tree3Scale;
			for (size_t i = 0; i < tree3Positions.size(); i++)
			{
				glm::mat4 matrix = glm::translate(modelMatrix, positions.front());
				matrix = glm::scale(matrix, scales.front());
				shader->setMat4("model", matrix);
				tree3.Draw(*shader);
				positions.pop_front();
				scales.pop_front();
			}
		}

		list <glm::vec3> positions = flowerPositions;
		for (size_t i = 0; i < flowerPositions.size(); i++)
		{
			glm::mat4 matrix = glm::translate(modelMatrix, positions.front());
			shader->setMat4("model", matrix);
			flower.Draw(*shader);
			positions.pop_front();
		}

		positions = bush1Positions;
		list <glm::vec3> scales = bush1Scale;
		for (size_t i = 0; i < bush1Positions.size(); i++)
		{
			glm::mat4 matrix = glm::translate(modelMatrix, positions.front());
			matrix = glm::scale(matrix, scales.front());
			shader->setMat4("model", matrix);
			bush1.Draw(*shader);
			positions.pop_front();
			scales.pop_front();
		}
	}

};

World::World()
{
}

World::~World()
{
}

