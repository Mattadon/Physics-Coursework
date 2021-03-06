/*
 * Program to demonstrate elastic collisions in a 3d cube
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "include/World.h"

#include "include/CubeGeometry.h"
#include "include/UVSphereGeometry.h"
#include "include/PlaneGeomtery.h"

#include "include/BulletWorld.h"
#include "include/BulletSphere.h"
#include "include/BulletPlane.h"
#include "include/BulletCube.h"

#include "include/TriangleMesh.h"
#include "include/PhysicsObject.h"

/* Keep track of the physics objects */
std::vector<PhysicsObject> MovingBits;
std::vector<PhysicsObject> StaticBits;

/* Screen parameters */
const int width = 800;
const int height = 800;

/* Functions to handle input */
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_movement(GLFWwindow *window, double xPos, double yPos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xpos, double ypos);

/* Stuff to read the mouse input to move the camera */
GLfloat lastX = width / 2.0;
GLfloat lastY = height / 2.0;

bool firstMouseInput = true;

//Mouse button flags
bool middleMouse = false;

//Key pressed flags
bool keys[1024];

//For calculating delta time
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

bool stillRunning = true;

int main( void )
{
    /* Attempt to initialise GLFW3, the window manager */
	if(!glfwInit())
	{
		std::cout << "Failed to start GLFW3" << std::endl;
		exit(EXIT_FAILURE);
	}

	/* Set up the main window */
	GLFWwindow* window = glfwCreateWindow(width, height, "Coursework 1", NULL, NULL);
	if(!window)
	{
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	/* Set the required callback functions */
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_movement);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	/* Set up GLEW before using any OpenGL functions */
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialise GLEW." << std::endl;
		return -1;
	}

	/* Tell OpenGL the size of the rendering window */
	glViewport(0, 0, width, height);

	/* Turn on depth testing to make stuff in front actually look like it's in front. */
	glEnable(GL_DEPTH_TEST);

    /*Draw wireframes */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	/* Load the shader program */
    Shader unshadedShader("shader/UnshadedDefault.vert", "shader/UnshadedDefault.frag");

    /* Some colours to use later */
    GLfloat red[3] = {1.0f, 0.0f, 0.0f};
    GLfloat yellow[3] = {1.0f, 1.0f, 0.0f};
    GLfloat blue[3] = {0.0f, 0.2f, 1.0f};
    GLfloat white[3] = {1.0f, 1.0f, 1.0f};

    /* Create some physics objects to add to the simulation */
    BulletWorld* world = new BulletWorld((float) GRAVITY);

    /* Create 2 sphere objects*/
	int segments = 10;
	int rings = 10;
	double radius = 0.5f;

    TriangleMesh sphereMesh(GetSpherePhong(segments, rings, radius), red);
    BulletSphere sphereBody(radius, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-5.0f, -4.0f, -5.0f), world);
    PhysicsObject sphereObject(&sphereMesh, &sphereBody);
    MovingBits.push_back(sphereObject);

    TriangleMesh sphereMesh2(GetSpherePhong(segments, rings, radius), blue);
    BulletSphere sphereBody2(radius, 1.0f, glm::vec3(3.0f, 3.0f, 0.0f), glm::vec3(-2.0f, -2.0f, -1.0f), world);
    PhysicsObject sphereObject2(&sphereMesh2, &sphereBody2);
    MovingBits.push_back(sphereObject2);

    /* Create a cube */
    float edgeLength = 1.0f;

    TriangleMesh cubeMesh(GetCubeGeometry(edgeLength), yellow);
    BulletCube cubeBody(edgeLength, 1.0f, glm::vec3(-3.0f, -3.0f, 0.0f), glm::vec3(5.0f, 5.0f, -1.0f), world);
    PhysicsObject cubeObject(&cubeMesh, &cubeBody);
    MovingBits.push_back(cubeObject);

    /* Create the container box */
    float boxEdge = 10.0f;
    TriangleMesh planeMesh(GetPlaneGeometry(boxEdge, boxEdge), white);

    BulletPlane bottomBody(glm::vec3(0.0f, 1.0f, 0.0f), -boxEdge/2, world);
    PhysicsObject bottomObject(&planeMesh, glm::vec3(0.0f, -boxEdge/2, 0.0f), glm::vec3((float) PIo2, 0.0f, 0.0f), &bottomBody);
    StaticBits.push_back(bottomObject);

    BulletPlane leftBody(glm::vec3(1.0f, 0.0f, 0.0f), -boxEdge/2, world);
    PhysicsObject leftObject(&planeMesh, glm::vec3(-boxEdge/2, 0.0f, 0.0f), glm::vec3(0.0f, (float) PIo2, 0.0f), &bottomBody);
    StaticBits.push_back(leftObject);

    BulletPlane backBody(glm::vec3(0.0f, 0.0f, 1.0f), -boxEdge/2, world);
    PhysicsObject backObject(&planeMesh, glm::vec3(0.0f, 0.0f, -boxEdge/2), glm::vec3(0.0f, 0.0f, 0.0f), &bottomBody);
    StaticBits.push_back(backObject);

    BulletPlane rightBody(glm::vec3(-1.0f, 0.0f, 0.0f), -boxEdge/2, world);
    PhysicsObject rightObject(&planeMesh, glm::vec3(boxEdge/2, 0.0f, 0.0f), glm::vec3(0.0f, (float) PIo2, 0.0f), &bottomBody);
    StaticBits.push_back(rightObject);

    BulletPlane frontBody(glm::vec3(0.0f, 0.0f, -1.0f), -boxEdge/2, world);
    PhysicsObject frontObject(&planeMesh, glm::vec3(0.0f, 0.0f, boxEdge/2), glm::vec3(0.0f, 0.0f, 0.0f), &bottomBody);
    StaticBits.push_back(frontObject);

    BulletPlane topBody(glm::vec3(0.0f, -1.0f, 0.0f), -boxEdge/2, world);
    PhysicsObject topObject(&planeMesh, glm::vec3(0.0f, boxEdge/2, 0.0f), glm::vec3((float) PIo2, 0.0f, 0.0f), &bottomBody);
    StaticBits.push_back(topObject);


    /* Main loop */
	while(!glfwWindowShouldClose(window) && stillRunning)
	{
	    //Calculate the time since the last frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		/* Rendering commands */
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Black
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Generate the view matrix */
		glm::mat4 view;
		view = camera.GetViewMatrix();
		/* Generate the projection matrix */
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.Fov), (GLfloat)width / (GLfloat)width, 0.1f, 100.0f);

		world->stepWorld(1.0f / 60.0f);

        unshadedShader.Use();

        /* Draw the moving bits */
        for(int i = 0; i < MovingBits.size(); i++)
        {
            MovingBits[i].Draw(unshadedShader, view, projection);
        }
        /* Draw the static bits */
        for(int i = 0; i < StaticBits.size(); i++)
        {
            StaticBits[i].Draw(unshadedShader, view, projection);
        }

		glfwSwapBuffers(window);
	}

	/* Terminate properly */
	glfwTerminate();
	return 0;
}

/*
* Record the states of keys when one changes
*/
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//Check to see if a new key has been pressed or released
	if (action == GLFW_PRESS)
	{
		keys[key] = true;

        if(keys[GLFW_KEY_Q] || keys[GLFW_KEY_ESCAPE])
            stillRunning = false; //Set the flag to close next frame
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = false;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.scroll_input(yoffset);
}

/*
* Record the changes in position of the mouse, use it to update the camera
*/
void mouse_movement(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouseInput)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouseInput = false;
	}

	//Difference between current moose position and previous
	GLfloat deltaX = xPos - lastX;
	GLfloat deltaY = lastY - yPos;

	//Update previous
	lastX = xPos;
	lastY = yPos;


	if (middleMouse)
	{
		if (keys[GLFW_KEY_LEFT_SHIFT])
			camera.pan_camera(deltaX, deltaY);
		else
			camera.move_camera(deltaX, deltaY);
	}
}

/*
* Record the state of the middle mouse when it changes
*/
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
		middleMouse = true;
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
		middleMouse = false;
}



