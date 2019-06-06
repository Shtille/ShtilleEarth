#version 330 core

uniform mat4 u_projection_view_model;

uniform vec4  u_stuv_scale;
uniform vec4  u_stuv_position;
uniform float u_planet_radius;
//uniform float u_planet_height;
uniform float u_skirt_height;

uniform mat3 u_face_transform;

layout(location = 0) in vec3 a_position;

out vec2 v_texcoord;

void main()
{
	// Vector is laid out as (s, t, u, v)
	vec4 stuv_point = a_position.xyxy * u_stuv_scale + u_stuv_position;

	vec3 face_point = normalize(u_face_transform * vec3(stuv_point.xy, 1.0));

	float height = 0.0;//u_planet_height * texture2DLod(heightMap, stuv_point.zw, 0.0).x;
	float skirt_height = a_position.z * u_skirt_height;
	vec3 sphere_point = face_point * (u_planet_radius + height + skirt_height);
    
	gl_Position = u_projection_view_model * vec4(sphere_point, 1.0);

	v_texcoord = stuv_point.zw;
}