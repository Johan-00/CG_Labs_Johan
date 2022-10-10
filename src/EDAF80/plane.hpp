#pragma once

#include "core/helpers.hpp"
#include "core/node.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

//struct SpinConfiguration
//{
//	float axial_tilt{ 0.0f }; //!< Angle in radians between the body's rotational and orbital axis.
//	float speed{ 0.0f };      //!< Rotation speed in radians per second.
//};

class Plane
{
public:
	//! \brief Default constructor for a plane.
	//!
	//! @param [in] 
	//!             
	//! @param [in] 
	//!             
	//! @param [in] 
	//!             
	Plane(glm::vec3 initialpotition);

	void pitch(double angle);

	void roll(double angle);

	void yaw(double angle);

	void fly(double deltaTime);

	glm::vec3 getPosition();

	glm::vec3 getDirection();

	glm::vec3 getVertical();

private:
	struct {
		glm::vec3 position;
		glm::vec3 forward;
		glm::vec3 up;
		glm::vec3 right;
		double speed;
	} plane_data;

};
