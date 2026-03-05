#include "camera.h"

Camera::Camera(
	glm::vec3 cameraPos,
	glm::vec3 cameraFront,
	glm::vec3 cameraUp)
: cameraPos(cameraPos), cameraFront(cameraFront), cameraUp(cameraUp)
{
	std::cout << "Camera created" << '\n';	
};

void Camera::update()
{
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 Camera::getViewMatrix()
{
	return view;
}

// Based on mouse move
void Camera::move(double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.f) pitch = 89.f;
	if (pitch < -89.f) pitch = -89.f;
	
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

// Based on key press
void Camera::move(CameraDirection direction)
{
	switch (direction)
	{
		case FORWARD:
			cameraPos += cameraSpeed * cameraFront;
			break;
		case BACKWARD:
			cameraPos -= cameraSpeed * cameraFront;
			break;
		case LEFT:
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		case RIGHT:
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
	}	
}


