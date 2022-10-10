#include "plane.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "core/helpers.hpp"
#include "core/Log.h"

Plane::Plane(glm::vec3 initialpotition)
{
	plane_data.position = initialpotition;
	plane_data.forward= glm::vec3(0.f,0.0f,1.0f);
	plane_data.up= glm::vec3(0.f, 1.0f, 0.0f);
	plane_data.right = glm::vec3(1.f, 0.0f, 0.0f);
	plane_data.speed=2.0;
}	
	
void Plane::pitch(double angle)
{
	plane_data.forward = glm::normalize(plane_data.forward * (float) std::cos(angle)+ plane_data.up * (float) std::sin(angle));
	plane_data.up = glm::cross(plane_data.right , plane_data.forward);
}

void Plane::roll(double angle)
{
	plane_data.right = glm::normalize(plane_data.right * (float) std::cos(angle) + plane_data.up * (float) std::sin(angle));
	plane_data.up = glm::cross(plane_data.right, plane_data.forward);
}

void Plane::yaw(double angle)
{
	plane_data.right = glm::normalize(plane_data.right * (float) std::cos(angle) + plane_data.forward * (float) std::sin(angle));
	plane_data.forward = glm::cross(plane_data.up, plane_data.right);
}

void Plane::fly(double deltaTime)
{
	plane_data.position += (float)plane_data.speed * plane_data.forward*(float)deltaTime;

}

glm::vec3 Plane::getPosition(){
	return plane_data.position;
}

glm::vec3 Plane::getDirection(){
	return plane_data.forward;
}

glm::vec3 Plane::getVertical() {
	return plane_data.up;
}
