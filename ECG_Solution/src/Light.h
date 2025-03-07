#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <learnopengl/shader_m.h>
using namespace glm;


class Light
{
private:
	float cutOff = 0.0f;
	float outerCutOff = 0.0f;

public:

	enum lightTypes {
		directional,
		point,
		spot,
		none
	};

	vec3 lightDirection = vec3(0.0f);
	vec3 lightPos = vec3(0.0f);
	vec3 lightColor= vec3(0.0f);
	lightTypes lightType = none;

	Light();
	~Light();

	void setDirectionalLight(vec3 color, vec3 direction) {
		lightType = directional;

		lightColor = color;
		lightDirection = direction;
	}

	void setPointLight(vec3 pos, vec3 color) {
		lightType = point;

		lightPos = pos;
		lightColor = color;
	}

	void setSpotLight(vec3 pos, vec3 color, vec3 direction, float cutOff, float outerCutOff) {
		lightType = spot;

		lightPos = pos;
		lightColor = color;
		lightDirection = direction;

		this->cutOff = cos(radians(cutOff));
		this->outerCutOff = cos(radians(outerCutOff));
	}

	void toShader(Shader* shader, int counter = 0) {
		shader->use();
		std::string s = to_string(counter);
		if (lightType == directional) {
			shader->setVec3("dirLight.direction", lightDirection);
			shader->setVec3("dirLight.lightColor", lightColor);
		}
		else if (lightType == point) {
			shader->setVec3("pointLights["+ s + "].lightPos", lightPos);
			shader->setVec3("pointLights["+ s + "].lightColor", lightColor);
			shader->setInt("numberOfPointLights", counter + 1);
		}
		else if (lightType == spot) {
			shader->setVec3("spotLights[" + s + "].lightPos", lightPos);
			shader->setVec3("spotLights[" + s + "].direction", lightDirection);
			shader->setFloat("spotLights[" + s + "].cutOff", cutOff);
			shader->setFloat("spotLights[" + s + "].outerCutOff", outerCutOff);
		}
	}
};

Light::Light()
{
}

Light::~Light()
{
}