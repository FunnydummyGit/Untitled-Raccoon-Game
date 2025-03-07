#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in ivec4 boneIds; 
layout (location = 4) in vec4 weights;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool hasWeights = false; 

uniform bool shadowBool = true;

void main()
{
    if(shadowBool){
        vec4 totalPosition = vec4(0.0f);
        vec3 localNormal = vec3(0.0f);
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(boneIds[i] == -1) 
                continue;
            if(boneIds[i] >=MAX_BONES) 
            {
                totalPosition = vec4(aPos,1.0f);
                break;
            }
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos,1.0f);
            totalPosition += localPosition * weights[i];
        }
 
        if(totalPosition != vec4(0.0f, 0.0f, 0.0f, 0.0f)){
             gl_Position = lightSpaceMatrix * model * totalPosition;
        } else {
             gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
        }
    }
}

