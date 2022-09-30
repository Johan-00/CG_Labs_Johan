#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	auto x_vec = glm::vec2(1.0f, x);
	auto b = glm::mat2(1.0f, -1.0f, 0.0f, 1.0f);
	auto p_vec = glm::mat3x2(p0.x, p1.x, p0.y, p1.y, p0.z, p1.z);

	auto p_x = x_vec * b * p_vec;

	return p_x;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
	auto x_vec = glm::vec4(1.0f, x, x * x, x * x * x);

	auto t_vec = glm::transpose(glm::mat4(0.0f, 1.0f, 0.0f, 0.0f,
		-t, 0.0f, t, 0.0f,
		2.0f * t, t - 3.0f, 3.0f - 2.0f * t, -t,
		-t, 2.0f - t, t - 2.0f, t));

	auto p_vec = glm::mat3x4(p0.x, p1.x, p2.x, p3.x,
		p0.y, p1.y, p2.y, p3.y,
		p0.z, p1.z, p2.z, p3.z);

	auto q_x = x_vec * t_vec * p_vec;

	return q_x;
}
