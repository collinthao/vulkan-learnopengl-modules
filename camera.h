#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <chrono>
#include <iostream>



enum CameraDirection 
{
	FORWARD,
	BACKWARD,
	RIGHT,
	LEFT
};

class Camera
{
private:
	glm::mat4 view;
	float sensitivity = 0.1f;
public:
	Camera(
	glm::vec3 cameraPos,
	glm::vec3 cameraFront,
	glm::vec3 cameraUp);

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	float yaw = -90.f;
	float pitch = 0.f;
	float lastX = WIDTH/2;		
	float lastY = HEIGHT/2;		
	float cameraSpeed = 5.f;		
	bool firstMouse = true;
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	glm::mat4 getViewMatrix();
	void update();
	void move(double xpos, double ypos);
	void move(CameraDirection direction);
};
