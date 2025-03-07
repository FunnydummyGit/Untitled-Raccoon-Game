
#include "PxPhysicsAPI.h"
#include <characterkinematic/PxControllerManager.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/model_animation.h> //change from model.h to model_animation.h
#include <learnopengl/animator.h> //added

#include <INIReader.h>

#include <iostream>
#include <list>

#include "Camera1.h"
#include "Camera2.h"
#include "Hitbox.h"
#include "PlayerController.h"
#include "KomplexStaticBody.h"
#include "StaticHitbox.h"
#include "Level.h"
#include "TextClass.h"
#include "Border.h"
#include "Light.h"
#include "World.h"



/* --------------------------------------------- */
// Prototypes
/* --------------------------------------------- */

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void setPerFrameUniforms(Shader* shader, Camera& camera, DirectionalLight& dirL, PointLight& pointL);
Camera cameraKeyboard(Camera camera, float dt);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
int rayCasting(double x, double y, list<Hitbox> objects, Camera camera);
bool intersectSphere(const glm::vec3& p, const glm::vec3& d, float r, float& t1, float& t2);
void BuildingObjectsToShader(list <Hitbox>* objects, Shader* shader, list <Model>* models);
void StaticObjectsToShader(list <KomplexStaticBody>* objects, Shader* shader, list <Model>* models);
list <Hitbox> updateAllModelMatrix(list <Hitbox> objects);
void loadScene(list <Hitbox>* objects, list <KomplexStaticBody>* staticObjects, PxScene* mScene, Border* border);
glm::mat4 calculate_lookAt_matrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp);
void renderScene(Shader* shader, Animator* animator, World* world, Model* raccoon, Border* border, 
	Model* borderModel, Model* goalModel, PlayerController* PlayerRaccoon,
	list <Model>* StaticModelList, list <Model>* ObjectsList, list <KomplexStaticBody>* StaticObjetcs, 
	list <Hitbox>* BuildingObjects, Goal* goal, bool shadow);




/* --------------------------------------------- */
// Global variables
/* --------------------------------------------- */

static bool start = true;
static bool restart = false;
static bool won = false;
static bool phase2 = false;
static bool phasebeginn = false;
static bool info = false;
static bool _wireframe = false;
static bool _culling = true;
static bool _dragging = false;
static bool _strafing = false;
static bool _forward = false;
static bool _backward = false;
static bool _right = false;
static bool _left = false;
static bool _up = false;
static bool _running = false;
static bool _raycast = false;
static bool _day = false;


float scroll_offset = 0.0f;
float fov = 60.0f;
double mouseX, mouseY = 0.0;
double lastMouseX, lastMouseY = 0.0;
int keyCounter = 0;

int level = 1;
int lastLevel = 2;

physx::PxPhysics* mPhysics = NULL;

Hitbox* selected{};

Text winText, subText;

// settings
unsigned int SCR_WIDTH = 1900;
unsigned int SCR_HEIGHT = 1080;


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

	/* --------------------------------------------- */
	// Main
	/* --------------------------------------------- */
	int main()
	{
		//read Settings
		INIReader reader("assets/settings.ini");

		SCR_WIDTH = reader.GetInteger("window", "width", 1920);
		SCR_HEIGHT = reader.GetInteger("window", "height", 1080);
		int refresh_rate = reader.GetInteger("window", "refresh_rate", 60);
		bool frame_cap = reader.GetBoolean("window", "frame_cap", false);
		bool fullscreen = reader.GetBoolean("window", "fullscreen", true);
		std::string title = reader.Get("window", "title", "Unitiled Raccoon Game");
		float brightness = reader.GetReal("window", "brightness", 1.0f);

		fov = reader.GetReal("camera", "fov", 60.0);
		float cameraNear = reader.GetReal("camera", "near", 0.1);
		float cameraFar = reader.GetReal("camera", "far", 100.0);




		// glfw: initialize and configure
		// ------------------------------
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);


#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		
		// glfw window creation
		// --------------------
		// Open window
		GLFWmonitor* monitor = nullptr;
		if (fullscreen)
			monitor = glfwGetPrimaryMonitor();
		GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, title.c_str(), monitor, nullptr);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return -1;
		}
		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetKeyCallback(window, key_callback);
		glfwSetScrollCallback(window, scroll_callback);


		// glad: load all OpenGL function pointers
		// ---------------------------------------
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return -1;
		}

		// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
		stbi_set_flip_vertically_on_load(true);


		// configure global opengl state
		// -----------------------------
		glEnable(GL_DEPTH_TEST);

		//PhysicsSetup
		PxReal mAccumulator = 0.0f;
		PxReal mStepSize = 1.0f / refresh_rate;

		physx::PxDefaultAllocator mDefaultAllocatorCallback;
		physx::PxDefaultErrorCallback mDefaultErrorCallback;
		physx::PxDefaultCpuDispatcher* mDispatcher = NULL;
		physx::PxTolerancesScale mToleranceScale;
		physx::PxPhysics* mPhysics = NULL;

		physx::PxFoundation* mFoundation = NULL;

		physx::PxScene* mScene = NULL;

		physx::PxControllerManager* manager = NULL;

		physx::PxPvd* mPvd = NULL;

		//init physx
		mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
		if (!mFoundation) throw("PxCreateFoundation failed!");
		mPvd = PxCreatePvd(*mFoundation);

		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
		//mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(),true, mPvd);
		mToleranceScale.length = 100;        // typical length of an object
		mToleranceScale.speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice
		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, mPvd);
		//mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale);
		physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = mDispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		mScene = mPhysics->createScene(sceneDesc);

		physx::PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}

		manager = PxCreateControllerManager(*mScene);
		manager->setOverlapRecoveryModule(true);

		//Level loading
		std::string LevelPath = "assets/Level/Level" + to_string(level) + ".ini";
		Level lvl(LevelPath, mPhysics, SCR_WIDTH, SCR_HEIGHT);

		// camera
		Camera camera(lvl.cameraPosition, fov, float(SCR_WIDTH), float(SCR_HEIGHT), cameraNear, cameraFar, -90.0f, 0.0f);
		Camera2 camera2(fov, float(SCR_WIDTH), float(SCR_HEIGHT), cameraNear, cameraFar, 0.0f);

		// build and compile shaders
		// -------------------------
		//Shader ourShader("assets/shader/shader.vs", "assets/shader/shader.fs");
		Shader ourShader("assets/shader/shader.vs", "assets/shader/shader.fs");
		Shader shadowShader("assets/shader/shadowDepthMapping.vs", "assets/shader/shadowDepthMapping.fs");
		Shader shadowCubeShader("assets/shader/shadowDepthCubeMapping.vs", "assets/shader/shadowDepthCubeMapping.fs", "assets/shader/shadowDepthCubeMapping.gs");
		winText.init(SCR_WIDTH, SCR_HEIGHT,"assets/fonts/M PLUS Rounded 1c/MPLUSRounded1c-Bold.ttf");
		subText.init(SCR_WIDTH, SCR_HEIGHT, "assets/fonts/Bitter/static/Bitter-Medium.ttf");

		ourShader.use();
		ourShader.setFloat("brightness", brightness);

		// configure depth map FBO
	// -----------------------
		const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
		unsigned int depthMapFBO;
		glGenFramebuffers(1, &depthMapFBO);

		//create depth map
		unsigned int depthMap;
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		unsigned int depthCubeMapFBO;
		glGenFramebuffers(1, &depthCubeMapFBO);
		// create depth cubemap texture
		unsigned int depthCubemap;
		glGenTextures(1, &depthCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		ourShader.use();
		ourShader.setInt("diffuseTexture", 0);
		ourShader.setInt("metallicMap", 1);
		ourShader.setInt("normalMap", 2);
		ourShader.setInt("roughnessMap", 3);
		ourShader.setInt("aoMap", 4);
		ourShader.setInt("shadowMap", 99);
		ourShader.setInt("depthCubeMap", 100);


		//Adding Objects to Scene
		Model raccoon("assets/Obj/Raccoon/raccoon_with_armature.dae");
		Animation raccoonWalk("assets/Obj/Raccoon/raccoon_with_armature.dae", &raccoon);
		Animator animator(&raccoonWalk);

		list <Model> ObjectsList = lvl.ObjectModelList;
		list <Hitbox> BuildingObjects = lvl.BuildingObjects;
		list <Model> StaticModelList = lvl.StaticModelList;
		list <KomplexStaticBody> StaticObjetcs = lvl.StaticObjects;
		Goal goal = lvl.goal;
		Model goalModel = lvl.goalModel.front();
		Model background = lvl.Background.front();
		Model sky = lvl.Sky.front();
		Model skyDay = lvl.SkyDay.front();
		Model borderModel = lvl.Background.back();
		glm::mat4 backgroundMatrix = lvl.backgroundM;
		glm::vec3 worldTranslate = lvl.worldTranslate;
		glm::vec3 worldScale = lvl.worldScale;
		Border border = lvl.border;
		Light sun = lvl.sun;
		sun.toShader(&ourShader);

		World world;
		world.generate(backgroundMatrix, worldTranslate, worldScale);

		list <Light> PointLights = lvl.pointLight;
		list <Light> treshholdPointLights = PointLights;
		for (size_t i = 0; i < lvl.numberOfPointLights; i++)
		{
			treshholdPointLights.front().toShader(&ourShader);
			treshholdPointLights.pop_front();
		}

		list <Light> SpotLights = lvl.Spotlights;
		list <Light> treshholdSpotLights = SpotLights;
		for (size_t i = 0; i < lvl.numberOfSpotLights; i++)
		{
			treshholdSpotLights.front().toShader(&ourShader);
			treshholdSpotLights.pop_front();
		}

		physx::PxMaterial* mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
		physx::PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, physx::PxPlane(0, 1, 0, 1), *mMaterial);
		mScene->addActor(*groundPlane);

		PlayerController PlayerRaccoon; 
		PlayerRaccoon.createPlayerController(mPhysics,manager, 1.25f, 1.85 ,2.6f, lvl.playerPosition, lvl.playerYaw);

		loadScene(&BuildingObjects, &StaticObjetcs, mScene, &border);
		animator.UpdateAnimation(0.0f);


		int framecounter = 0;
		int frameCounterLast;
		float timer = 0.0f;
		// FreeType
	// --------

		while (!glfwWindowShouldClose(window))
		{
			// per-frame time logic
			// --------------------
			float currentFrame = static_cast<float>(glfwGetTime());
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			timer += deltaTime;
			framecounter++;
			if (timer >= 1.0f) {
				timer = 0.0f;
				frameCounterLast = framecounter;
				framecounter = 0.0;
			}

			lastMouseX = mouseX;
			lastMouseY = mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);

			// input
			// -----


			//restart Scene
			if (restart) {
				phase2 = false;
				phasebeginn = true;
				mScene->release();
				mScene = mPhysics->createScene(sceneDesc);

				physx::PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
				if (pvdClient)
				{
					pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
					pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
					pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
				}

				manager = PxCreateControllerManager(*mScene);
				manager->setOverlapRecoveryModule(true);

				mScene->addActor(*groundPlane);

				Level newlvl(LevelPath, mPhysics, SCR_WIDTH, SCR_HEIGHT);
				lvl = newlvl;

				camera.setPosition(lvl.cameraPosition);
				PlayerRaccoon.release();
				PlayerRaccoon.createPlayerController(mPhysics, manager, 1.25f, 1.85, 2.5f, lvl.playerPosition, lvl.playerYaw);

				ObjectsList = lvl.ObjectModelList;
				BuildingObjects = lvl.BuildingObjects;
				StaticModelList = lvl.StaticModelList;
				StaticObjetcs = lvl.StaticObjects;
				goal = lvl.goal;
				goalModel = lvl.goalModel.front();
				border = lvl.border;
				sun = lvl.sun;
				sun.toShader(&ourShader);

				list <Light> PointLights = lvl.pointLight;
				list <Light> treshholdPointLights = PointLights;
				for (size_t i = 0; i < lvl.numberOfPointLights; i++)
				{
					treshholdPointLights.front().toShader(&ourShader);
					treshholdPointLights.pop_front();
				}

				list <Light> SpotLights = lvl.Spotlights;
				list <Light> treshholdSpotLights = SpotLights;
				for (size_t i = 0; i < lvl.numberOfSpotLights; i++)
				{
					treshholdSpotLights.front().toShader(&ourShader);
					treshholdSpotLights.pop_front();
				}

				loadScene(&BuildingObjects, &StaticObjetcs, mScene, &border);
				restart = false;
				won = false;
			}

			mAccumulator += deltaTime;
			bool inFrame = false;

			if (goal.checkIfReached(PlayerRaccoon.position)) {
				won = true;
			}

			if (won) {
				if (phasebeginn && level < lastLevel) {
					phase2 = false;
					mScene->release();
					mScene = mPhysics->createScene(sceneDesc);

					physx::PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
					if (pvdClient)
					{
						pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
						pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
						pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
					}

					manager = PxCreateControllerManager(*mScene);
					manager->setOverlapRecoveryModule(true);

					mScene->addActor(*groundPlane);

					level++;

					LevelPath = "assets/Level/Level" + to_string(level) + ".ini";

					Level newlvl(LevelPath, mPhysics, SCR_WIDTH, SCR_HEIGHT);
					lvl = newlvl;

					camera.setPosition(lvl.cameraPosition);
					PlayerRaccoon.release();
					PlayerRaccoon.createPlayerController(mPhysics, manager, 1.25f, 1.85, 2.5f, lvl.playerPosition, lvl.playerYaw);

					ObjectsList = lvl.ObjectModelList;
					BuildingObjects = lvl.BuildingObjects;
					StaticModelList = lvl.StaticModelList;
					StaticObjetcs = lvl.StaticObjects;
					goal = lvl.goal;
					goalModel = lvl.goalModel.front();
					border = lvl.border;
					sun = lvl.sun;
					sun.toShader(&ourShader);

					list <Light> PointLights = lvl.pointLight;
					ourShader.setInt("numberOfPointLights", lvl.numberOfPointLights);
					list <Light> treshholdPointLights = PointLights;
					for (size_t i = 0; i < lvl.numberOfPointLights; i++)
					{
						treshholdPointLights.front().toShader(&ourShader);
						treshholdPointLights.pop_front();
					}

					list <Light> SpotLights = lvl.Spotlights;
					ourShader.setInt("numberOfSpotLights", lvl.numberOfSpotLights);
					list <Light> treshholdSpotLights = SpotLights;
					for (size_t i = 0; i < lvl.numberOfSpotLights; i++)
					{
						treshholdSpotLights.front().toShader(&ourShader);
						treshholdSpotLights.pop_front();
					}

					loadScene(&BuildingObjects, &StaticObjetcs, mScene, &border);
					restart = false;
					won = false;
				}
			}
			else
			{
				if (mAccumulator >= mStepSize)
				{
					inFrame = true;
					mAccumulator -= mStepSize;
					mScene->simulate(mStepSize);
					mScene->fetchResults(true);
					if (!phase2)
					{
						if (phasebeginn) {
							glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
							phasebeginn = false;
						}
						camera = cameraKeyboard(camera, mStepSize);
					}
					else
					{
						if (phasebeginn) {
							glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
							phasebeginn = false;
						}

						bool moved = false;

						if (_forward == true) {
							PlayerRaccoon.move(PlayerRaccoon.FORWARD, _running);
							moved = true;
						}
						if (_backward == true) {
							PlayerRaccoon.move(PlayerRaccoon.BACKWARD, _running);
							moved = true;
						}
						if (_right == true) {
							PlayerRaccoon.move(PlayerRaccoon.RIGHT, _running);
							moved = true;
						}
						if (_left == true) {
							PlayerRaccoon.move(PlayerRaccoon.LEFT, _running);
							moved = true;
						}
						if (_up == true) {
							PlayerRaccoon.move(PlayerRaccoon.UP, _running);
						}

						PlayerRaccoon.updatePlayerCharacter(mStepSize, _running);

						//added for skelletal Animation
						//----------------------------------------------------------------------------------------------------------------------------------------
						if (moved)
							animator.UpdateAnimation(mStepSize);
						else {
							animator.PlayAnimation(&raccoonWalk);
							animator.UpdateAnimation(0.0f);
						}

						//----------------------------------------------------------------------------------------------------------------------------------------

					}
				}
				
				//Camera Rotation
				if (!phase2) {
					camera.updateCamera(mouseX, mouseY, lastMouseX, lastMouseY, _dragging);
				}
				else
				{
					PlayerRaccoon.rotate(mouseX, mouseY, lastMouseX, lastMouseY, SCR_WIDTH, SCR_HEIGHT);
					camera2.updateCamera(&PlayerRaccoon);
				}

				if (_strafing) {

					list<Hitbox> next = BuildingObjects;
					if (!_raycast)
					{
						int j = rayCasting(mouseX, mouseY, BuildingObjects, camera);
						if (j >= 0)
						{

							for (size_t i = 0; i < j; i++)
							{
								next.pop_front();
							}
							selected = &next.front();
							_raycast = true;

						}
					}
					if (_raycast) {
						selected->mouseInput(mouseX, mouseY, lastMouseX, lastMouseY, scroll_offset, camera);
						_raycast = false;
						scroll_offset = 0.0f;
					}
				}
				else
				{
					_raycast = false;
				}
			}

			BuildingObjects = updateAllModelMatrix(BuildingObjects);
		// --------------------------------------------------------------
		//Using the Shader
		// --------------------------------------------------------------
			if ((inFrame || !frame_cap) || won) {
				// render
				// ------
				glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// 1. render depth of scene to texture(from light's perspective)
				// --------------------------------------------------------------
				glm::mat4 lightProjection, lightView;
				glm::mat4 lightSpaceMatrix;
				float near_plane = 1.0f, far_plane = 100.0f;
				if (_day) {
					//lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
					lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
					glm::vec3 shadowRenderPoint;
					if (phase2) {
						shadowRenderPoint = PlayerRaccoon.position + PlayerRaccoon.Front * glm::vec3(25.0f);
					}
					else
					{
						shadowRenderPoint = camera.position + camera.Front * glm::vec3(20.0f);
					}
					lightView = calculate_lookAt_matrix(sun.lightDirection + shadowRenderPoint, glm::vec3(0.0f, 0.0f, 0.0f) + shadowRenderPoint, glm::vec3(0.0, 1.0, 0.0));
					lightSpaceMatrix = lightProjection * lightView;
					// render scene from light's point of view
					shadowShader.use();
					shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

					glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
					glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
					glClear(GL_DEPTH_BUFFER_BIT);
					renderScene(&shadowShader, &animator, &world, &raccoon, &border, &borderModel, &goalModel, &PlayerRaccoon,
						 &StaticModelList, &ObjectsList, &StaticObjetcs, &BuildingObjects, &goal, true);
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}
				// 2. create depth cubemap transformation matrices
				// -----------------------------------------------
				else
				{
					near_plane = 1.0f, far_plane = 80.0f;
					glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
					std::vector<glm::mat4> shadowTransforms;
					//glm::mat4 shadowView = calculate_lookAt_matrix(PointLights.front().lightPos, PointLights.front().lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
					shadowTransforms.push_back(shadowProj * calculate_lookAt_matrix(PointLights.front().lightPos, PointLights.front().lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
					shadowTransforms.push_back(shadowProj * calculate_lookAt_matrix(PointLights.front().lightPos, PointLights.front().lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
					shadowTransforms.push_back(shadowProj * calculate_lookAt_matrix(PointLights.front().lightPos, PointLights.front().lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
					shadowTransforms.push_back(shadowProj * calculate_lookAt_matrix(PointLights.front().lightPos, PointLights.front().lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
					shadowTransforms.push_back(shadowProj * calculate_lookAt_matrix(PointLights.front().lightPos, PointLights.front().lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
					shadowTransforms.push_back(shadowProj * calculate_lookAt_matrix(PointLights.front().lightPos, PointLights.front().lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

					// 3. render scene to depth cubemap
					// --------------------------------
					glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
					glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
					glClear(GL_DEPTH_BUFFER_BIT);
					shadowCubeShader.use();
					for (unsigned int i = 0; i < 6; ++i)
						shadowCubeShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
					shadowCubeShader.setFloat("far_plane", far_plane);
					shadowCubeShader.setVec3("lightPos", PointLights.front().lightPos);
					renderScene(&shadowCubeShader, &animator, &world, &raccoon, &border, &borderModel, &goalModel, &PlayerRaccoon,
						 &StaticModelList, &ObjectsList, &StaticObjetcs, &BuildingObjects, &goal, true);
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}

				//4. Render Scene
		// --------------------------------------------------------------	
				// reset viewport
				glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// don't forget to enable shader before setting uniforms
				ourShader.use();

				ourShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
				glActiveTexture(GL_TEXTURE1 + 98);
				glBindTexture(GL_TEXTURE_2D, depthMap);
				glActiveTexture(GL_TEXTURE1 + 99);
				glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

				ourShader.setFloat("far_plane", far_plane);

				ourShader.setBool("day", _day);

				// view/projection transformation
				if (!phase2)
				{
					//glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
					//glm::mat4 projection = camera.projection;
					//glm::mat4 view = camera.getViewMatrix();
					//ourShader.setMat4("projection", shadowProj);
					//ourShader.setMat4("view", shadowView);
					//ourShader.setVec3("camPos", PointLights.front().lightPos);

					glm::mat4 projection = camera.projection;
					glm::mat4 view = camera.getViewMatrix();
					ourShader.setMat4("projection", projection);
					ourShader.setMat4("view", view);
					ourShader.setVec3("camPos", camera.position);
				}
				else
				{
					glm::mat4 projection = camera2.projection;
					glm::mat4 view = camera2.getViewMatrix();
					ourShader.setMat4("projection", projection);
					ourShader.setMat4("view", view);
					ourShader.setVec3("camPos", camera2.position);
				}

				ourShader.setMat4("model", backgroundMatrix);
				ourShader.setBool("Sky", true);
				if (_day) {
					skyDay.Draw(ourShader);
				}
				else
				{
					sky.Draw(ourShader);
				}
				ourShader.setBool("Sky", false);

				renderScene(&ourShader, &animator, &world, &raccoon, &border, &borderModel, &goalModel, &PlayerRaccoon,
					 &StaticModelList, &ObjectsList, &StaticObjetcs, &BuildingObjects, &goal, false);
				

				//glEnable(GL_BLEND);
				//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				//RenderText(textShader, "You Win", goal.position.x, goal.position.y, 0.25f, glm::vec3(0.5, 0.8f, 0.2f));
				//RenderText(ourShader, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
			
				if (level == 1) {
					glEnable(GL_CULL_FACE);
					subText.RenderText("Goal: Get to the Cupcake", (SCR_WIDTH / 100), (SCR_HEIGHT / 20) * 19, 0.5f, glm::vec3(1.0, 0.0, 1.0));
					subText.RenderText("I: control info ON/OFF", (SCR_WIDTH / 100), (SCR_HEIGHT / 20) * 18, 0.5f, glm::vec3(1.0, 0.0, 1.0));
					if (info) {
						if (!phase2) {
							subText.RenderText("WASD: Movement", (SCR_WIDTH / 100) * 80, (SCR_HEIGHT / 20) * 19, 0.5f, glm::vec3(1.0, 0.0, 1.0));
							subText.RenderText("RMB: rotate Camera", (SCR_WIDTH / 100) * 80, (SCR_HEIGHT / 20) * 18, 0.5f, glm::vec3(1.0, 0.0, 1.0));
							subText.RenderText("LMB: Dragging Objects ", (SCR_WIDTH / 100) * 80, (SCR_HEIGHT / 20) * 17, 0.5f, glm::vec3(1.0, 0.0, 1.0));
							subText.RenderText("LMB + Scroll: Pulling Objects", (SCR_WIDTH / 100) * 80, (SCR_HEIGHT / 20) * 16, 0.5f, glm::vec3(1.0, 0.0, 1.0));
							subText.RenderText("Enter: Playing-Mode", (SCR_WIDTH / 100) * 80, (SCR_HEIGHT / 20) * 15, 0.5f, glm::vec3(1.0, 0.0, 1.0));
						}
						else
						{
							subText.RenderText("WASD: Movement", (SCR_WIDTH / 100) * 80, (SCR_HEIGHT / 20) * 19, 0.5f, glm::vec3(1.0, 0.0, 1.0));
							subText.RenderText("Shift: Run", (SCR_WIDTH / 100) * 80, (SCR_HEIGHT / 20) * 18, 0.5f, glm::vec3(1.0, 0.0, 1.0));
							subText.RenderText("Space: Jump", (SCR_WIDTH / 100) * 80, (SCR_HEIGHT / 20) * 17, 0.5f, glm::vec3(1.0, 0.0, 1.0));
							subText.RenderText("Enter: Building-Mode", (SCR_WIDTH / 100) * 80, (SCR_HEIGHT / 20) * 16, 0.5f, glm::vec3(1.0, 0.0, 1.0));
						}
					}
					//std::string s = to_string(frameCounterLast);
					//subText.RenderText(s, (SCR_WIDTH / 100), (SCR_HEIGHT / 20) * 16, 0.5f, glm::vec3(1.0, 0.0, 1.0));
					glDisable(GL_CULL_FACE);
				}

				if (won) {
					glEnable(GL_CULL_FACE);
					winText.RenderText("YOU WIN", SCR_WIDTH / 5, SCR_HEIGHT / 2, 5.0f, glm::vec3(0.98, 0.62, 0.12));
					if (level < lastLevel)
						subText.RenderText("press Enter for next Level", SCR_WIDTH / 4, SCR_HEIGHT / 3, 1.5f, glm::vec3(1.0, 0.0, 1.0));
					subText.RenderText("press Backspace to restart", SCR_WIDTH / 4, SCR_HEIGHT / 4, 1.5f, glm::vec3(1.0, 0.0, 1.0));
					glDisable(GL_CULL_FACE);
				}

				// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
				// -------------------------------------------------------------------------------
				glfwSwapBuffers(window);
			}
			glfwPollEvents();
		}

		// glfw: terminate, clearing all previously allocated GLFW resources.
		// ------------------------------------------------------------------
		glfwTerminate();
		mScene->release();
		mPhysics->release();
		mFoundation->release();
		return 0;
	}




void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		_dragging = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		_dragging = false;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		_strafing = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		_strafing = false;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	scroll_offset = float(yoffset) * 0.25f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// F1 - Wireframe
	// F2 - Culling
	// Esc - Exit
	// i - Info

	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		_forward = true;
		keyCounter++;
	}
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		_forward = false;
		keyCounter--;
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		_backward = true;
		keyCounter++;
	}
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		_backward = false;
		keyCounter--;
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		_left = true;
		keyCounter++;
	}
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		_left = false;
		keyCounter--;
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		_right = true;
		keyCounter++;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {  
		_right = false;
		keyCounter--;
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		_up = true;
	}
	else if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
		_up = false;
	}

	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
		_running = true;
	}
	else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
		_running = false;
	}

	if (key == GLFW_KEY_I && action == GLFW_RELEASE) {
		if (info) {
			info = false;
		}
		else info = true;
	}
	


	if (action != GLFW_RELEASE) return;

	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, true);
		break;
	case GLFW_KEY_F1:
		_wireframe = !_wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);
		break;
	case GLFW_KEY_F2:
		_culling = !_culling;
		if (_culling) glEnable(GL_CULL_FACE);
		else glDisable(GL_CULL_FACE);
		break;
	case GLFW_KEY_F3:
		_day = !_day;
		break;
	case GLFW_KEY_ENTER:
		if(!won)
		phase2 = !phase2;
		phasebeginn = true;
		break;
	case GLFW_KEY_BACKSPACE:
		restart = true;
		break;
	}
}

Camera cameraKeyboard(Camera camera, float dt) {
	if (_forward == true) {
		camera.ProcessKeyboard(camera.FORWARD, dt);
	}
	else if (_backward == true) {
		camera.ProcessKeyboard(camera.BACKWARD, dt);
	}
	if (_right == true) {
		camera.ProcessKeyboard(camera.RIGHT, dt);
	}
	else if (_left == true) {
		camera.ProcessKeyboard(camera.LEFT, dt);
	}
	return camera;
}



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

int rayCasting(double x, double y, list<Hitbox> objects, Camera camera) {
		glm::vec2 mouse = (glm::vec2(x,y) / glm::vec2(SCR_WIDTH, SCR_HEIGHT)) * 2.0f - 1.0f;
		mouse.y = -mouse.y; //origin is top-left and +y mouse is down

		glm::mat4 toWorld = glm::inverse(camera.projection * camera.view);
		//equivalent to camera.transform.inverse() * camera.projection.inverse() but faster

		glm::vec4 from = toWorld * glm::vec4(mouse, -1.0f, 1.0f);
		glm::vec4 to = toWorld * glm::vec4(mouse, 1.0f, 1.0f);

		from /= from.w; //perspective divide ("normalize" homogeneous coordinates)
		to /= to.w;

		int clickedObject = -1;
		float minDist = 99999.0f;

		list<Hitbox> next = objects;
		for (size_t i = 0; i < objects.size(); ++i)
		{
			float t1, t2;
			glm::vec3 direction = to - from;
			glm::vec3 f = from;
			if (intersectSphere(f - next.front().position, direction, next.front().radius, t1, t2))
			{
				//object i has been clicked. probably best to find the minimum t1 (front-most object)
				if (t1 < minDist)
				{
					minDist = t1;
					return i;
				}
			}
			next.pop_front();
		}
	return -1;
}

//ray at position p with direction d intersects sphere at (0,0,0) with radius r. returns intersection times along ray t1 and t2
bool intersectSphere(const glm::vec3& p, const glm::vec3& d, float r, float& t1, float& t2)
{
	float A = glm::dot(d, d);
	float B = 2.0f * glm::dot(d, p);
	float C = glm::dot(p, p) - r * r;

	float dis = B * B - 4.0f * A * C;

	if (dis < 0.0f)
		return false;

	float S = sqrt(dis);

	t1 = (-B - S) / (2.0f * A);
	t2 = (-B + S) / (2.0f * A);
	return true;
}

void BuildingObjectsToShader(list <Hitbox>* objects, Shader* shader, list <Model>* models) {
	list <Hitbox> nextO = *objects;
	list <Model> nextM = *models;
	int size = nextO.size();

	for (size_t i = 0; i < size; i++)
	{
		glm::mat4 stoneMatrix = glm::mat4(1);
		stoneMatrix = nextO.front().getModelMatrix();

		shader->setMat4("model", stoneMatrix);
		nextM.front().Draw(*shader);

		nextO.pop_front();
		nextM.pop_front();
	}
}

void StaticObjectsToShader(list <KomplexStaticBody>* objects, Shader* shader, list <Model>* models) {
	list <KomplexStaticBody> nextO = *objects;
	list <Model> nextM = *models;
	int size = nextO.size();

	for (size_t i = 0; i < size; i++)
	{
		glm::mat4 stoneMatrix = glm::mat4(1);
		stoneMatrix = nextO.front().getModelMatrix();
		if (!nextO.front().getShadow()) {
			nextO.front().drawModel(shader);
		}

		shader->setMat4("model", stoneMatrix);
		nextM.front().Draw(*shader);

		nextO.pop_front();
		nextM.pop_front();
	}
}

list <Hitbox> updateAllModelMatrix(list <Hitbox> objects) {
	list <Hitbox> oldList = objects;
	list <Hitbox> newList;
	for (size_t i = 0; i < objects.size(); i++)
	{
		oldList.front().updateModelMatrix();
		newList.push_back(oldList.front());
		oldList.pop_front();
	}
	return newList;
}

void loadScene(list <Hitbox>* objects, list <KomplexStaticBody>* staticObjects, PxScene* mScene, Border* border) {
	mScene->addActor(*border->plane1);
	mScene->addActor(*border->plane2);
	mScene->addActor(*border->plane3);
	mScene->addActor(*border->plane4);

	list <KomplexStaticBody> staticSelect = *staticObjects;
	int s = staticSelect.size();
	for (size_t i = 0; i < s; i++)
	{
		staticSelect.front().addActor(mScene);
		staticSelect.pop_front();
	}

	list <Hitbox> select = *objects;
	s = select.size();
	for (size_t i = 0; i < s; i++)
	{
		mScene->addActor(*select.front().body);
		select.pop_front();
	}


	lastFrame = static_cast<float>(glfwGetTime());
	start = true;
}

glm::mat4 calculate_lookAt_matrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp)
{
	// 1. Position = known
	// 2. Calculate cameraDirection
	glm::vec3 zaxis = glm::normalize(position - target);
	// 3. Get positive right axis vector
	glm::vec3 xaxis = glm::vec3 (0.0);
	if (glm::normalize(worldUp) == zaxis) {
		xaxis = glm::vec3(1.0, 0.0, 0.0);
	}
	else
	{
		xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
	}
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

void renderScene(Shader* shader, Animator* animator, World* world, Model* raccoon, Border* border, Model* borderModel, Model* goalModel, PlayerController* PlayerRaccoon, list <Model>* StaticModelList, list <Model>* ObjectsList, list <KomplexStaticBody>* StaticObjetcs, list <Hitbox>* BuildingObjects, Goal* goal, bool shadow) {
	//added for skelletal Animation
	//----------------------------------------------------------------------------------------------------------------------------------------
	auto transforms = animator->GetFinalBoneMatrices();
	for (int i = 0; i < transforms.size(); ++i)
		shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
	//-----------------------------------------------------------------------------------------------------------------------------------------

	//border.Draw(ourShader);
	shader->setMat4("model", glm::mat4(1));
	border->Draw(*shader, borderModel);
	// render the loaded model
	shader->setBool("raccoon", true);
	shader->setMat4("model", PlayerRaccoon->GetFinalModelMatrix());
	raccoon->Draw(*shader);
	shader->setBool("raccoon", false);

	StaticObjectsToShader(StaticObjetcs, shader, StaticModelList);
	BuildingObjectsToShader(BuildingObjects, shader, ObjectsList);

	shader->setMat4("model", goal->modelMatrix);
	goalModel->Draw(*shader);

	world->Draw(shader, shadow);
}