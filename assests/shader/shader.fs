#version 430 core
struct DirLight {
    vec3 direction;
    vec3 lightColor;
}; 

struct PointLight {
    vec3 lightPos;
    vec3 lightColor;

};

struct SpotLight {
    vec3 lightPos;
    vec3 direction;

    float cutOff;
    float outerCutOff;

    vec3 lightColor;
       
};


out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

// lights
uniform DirLight dirLight;
uniform int numberOfPointLights = 0;
uniform PointLight pointLights[4];
uniform int numberOfSpotLights = 0;
uniform SpotLight spotLights[4];

//shadow
uniform samplerCube depthCubeMap;
uniform sampler2D shadowMap;

// old texture
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_metalic1;

// material parameters
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;


//Camera Position
uniform vec3 camPos;

// if Sky
uniform bool Sky = false;

//Bools
uniform bool shadowBool = true;

uniform bool shading = false;

uniform bool day = true;

uniform bool raccoon = false;

//important values 
uniform float far_plane;

uniform float brightness = 1.0f;

const float PI = 3.14159265359;

vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(texture_normal1, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 DirectionalLightCalcShading(DirLight light, vec3 V, vec3 N, vec3 albedo, float roughness, float ao, float metallic, vec3 F0)
{
     // reflectance equation
    vec3 Lo = vec3(0.0);
    // calculate per-light radiance
    vec3 H = normalize(V + light.direction);
    vec3 radiance = light.lightColor;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, light.direction, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
       
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, light.direction), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, light.direction), 0.0);        

    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

    return Lo;
}
// ----------------------------------------------------------------------------
vec3 PointLightCalcShading(PointLight light, vec3 V, vec3 N, vec3 albedo, float roughness, float ao, float metallic, vec3 F0)
{
    // reflectance equation
    vec3 Lo = vec3(0.0);
    // calculate per-light radiance
    vec3 L = normalize(light.lightPos - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(light.lightPos - WorldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.lightColor * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
       
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again


    return Lo;
}
// ----------------------------------------------------------------------------
// calculates the color when using a directional light.
vec3 calcDirLight(DirLight light, bool raccoon){
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light.direction);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * light.lightColor;
    // specular
    vec3 viewDir = normalize(camPos - WorldPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir); 
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    if (raccoon){
        spec /= 3;
    }
    vec3 specular = spec * light.lightColor;

    return specular + diffuse;
}
// ----------------------------------------------------------------------------
vec3 calcPointLight(PointLight light, bool raccoon){
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light.lightPos - WorldPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * light.lightColor;
    // specular
    vec3 viewDir = normalize(camPos - WorldPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir); 
    spec = pow(max(dot(normal, halfwayDir), 0.0), 50.0);
    if(raccoon){
        spec /= 3;
    }
    vec3 specular = spec * light.lightColor;

    return specular + diffuse;
}
// ----------------------------------------------------------------------------
//Sahdow Calculations
// ----------------------------------------------------------------------------
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(dirLight.direction);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}
// ----------------------------------------------------------------------------
float ShadowCalculationPointLight(vec3 fragPos, PointLight light)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - light.lightPos;
    // use the fragment to light vector to sample from the depth map    
    // float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    // closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    // float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
    // PCF
    // float shadow = 0.0;
    // float bias = 0.05; 
    // float samples = 4.0;
    // float offset = 0.1;
    // for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    // {
        // for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        // {
            // for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            // {
                // float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r; // use lightdir to lookup cubemap
                // closestDepth *= far_plane;   // Undo mapping [0;1]
                // if(currentDepth - bias > closestDepth)
                    // shadow += 1.0;
            // }
        // }
    // }
    // shadow /= (samples * samples * samples);
    float shadow = 0.0;
    float bias = 0.5;
    int samples = 20;
    float viewDistance = length(camPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthCubeMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
}
// ----------------------------------------------------------------------------



void main()
{		

    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    if(!Sky){
        if(shading){
            vec3 albedo     = pow(texture(texture_diffuse1, TexCoords).rgb, vec3(2.2));
            float metallic  = texture(texture_metalic1, TexCoords).r;
            float roughness = texture(texture_specular1, TexCoords).r;
            float ao        = texture(texture_height1, TexCoords).r;

            vec3 N = getNormalFromMap();
            vec3 V = normalize(camPos - WorldPos);

            vec3 ambient = vec3(0.03) * albedo * ao;

            // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
            // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
            vec3 F0 = vec3(0.04); 
            F0 = mix(F0, albedo, metallic);

            vec3 Lo = vec3(0);

            //vec3 color = vec3(0);
            if(day){
            vec3 Lo = DirectionalLightCalcShading(dirLight, V, N, albedo, roughness, ao, metallic, F0);
            }
            else{
                for(int i = 0; i < min(numberOfPointLights, 4); i++) 
                {
                    Lo += PointLightCalcShading(pointLights[i], V, N, albedo, roughness, ao, metallic, F0);
                    //shadow += ShadowCalculationPointLight(FragPosLightSpace, pointLights[i]); 
                }
            }

            float shadow = 0.0;
            if (day){
                    shadow += ShadowCalculation(FragPosLightSpace) * 0.75 ;
            }   
            else{
                if (shadowBool){
                    for(int i = 0; i < numberOfPointLights; i++){
                        //if(shadow > 0 && ShadowCalculationPointLight(WorldPos, pointLights[i]) < 1){
                        //    shadow /= 2;
                        //}
                        //else if(shadow < 1 && ShadowCalculationPointLight(WorldPos, pointLights[i]) > 0){
                        //    shadow += shadow/2;
                        //}
                        //else if(shadow == 0 && ShadowCalculationPointLight(WorldPos, pointLights[i]) > 0){
                        //    shadow = 0.5;
                        //}
                        shadow += ShadowCalculationPointLight(WorldPos, pointLights[i]);
                    }
                }
            }            
            if(shadow > 1.0){
                shadow = 1.0;
            }
             
             color = ambient + Lo;

             //// HDR tonemapping
             color = color / (color + vec3(1.0));
             // gamma correct
             color = pow(color, vec3(1.0/2.2)) * 4.5 * (1.0 - shadow);
        }
        else{
            vec3 lightColor = vec3(0.3);
            // ambient
            vec3 ambient = 0.3 * lightColor;
            // diffuse
                
            vec3 difSpec = vec3(0.0f);

            if(day){
                difSpec += calcDirLight(dirLight, raccoon);
            }
            else{
                for(int i = 0; i < numberOfPointLights; i++){
                    difSpec += calcPointLight(pointLights[i], raccoon);
                }
            }

            // calculate shadow
            float shadow = 0.0f;
            if (day){
                shadow += ShadowCalculation(FragPosLightSpace) * 0.75 ;
            }
            else{
                if (shadowBool){
                    for(int i = 0; i < numberOfPointLights; i++){
                        //if(shadow > 0 && ShadowCalculationPointLight(WorldPos, pointLights[i]) < 1){
                        //    shadow /= 2;
                        //}
                        //else if(shadow < 1 && ShadowCalculationPointLight(WorldPos, pointLights[i]) > 0){
                        //    shadow += shadow/2;
                        //}
                        //else if(shadow == 0 && ShadowCalculationPointLight(WorldPos, pointLights[i]) > 0){
                        //    shadow = 0.5;
                        //}
                        shadow += ShadowCalculationPointLight(WorldPos, pointLights[i]);
                    }
                }
            }
            if(shadow > 1.0){
                shadow = 1.0;
            }
            
            if(!day){
                difSpec = difSpec / (numberOfPointLights);
            }

            vec3 lighting = (ambient + (1.0 - shadow) * (difSpec)) * color;    
            
            //float shadow = shadowCalc();                      
            //vec3 lighting = (ambient + (shadow) * (diffuse + specular)) * color;

            if(shadowBool)
            color = lighting;

            if(!shadowBool && !day){
                color = color * 1.2;
            }
        }
    }
    FragColor = vec4(color * brightness, 1.0f);
}