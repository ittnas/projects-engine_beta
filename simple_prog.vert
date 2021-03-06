#version 410

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;

//out vec4 camera_pos;
//out vec3 camera_normal;
out vec4 world_position;
out vec3 world_normal;
//out vec2 tex_coord_out;

layout(std140) uniform model_buffer {
  mat4 model_matrix;
  mat3 normal_matrix;
};

layout(std140) uniform view_buffer {
  mat4 view_matrix;
};

layout(std140) uniform projection_buffer {
  mat4 projection_matrix;
};
/*
struct LightInfo {
       vec4 position;
       vec4 intensity;
       vec3 direction;
       float exponent;
       float cutoff;
       float kc;
       float kl;
       float kq;
       bool is_active;
};

layout(std140) uniform light_buffer {
  LightInfo lights[4];
};
*/
/*layout(std140) uniform transform_buffer
{
	mat4 camera_to_clip_matrix;
	mat4 model_to_camera_matrix;
	mat4 view_matrix;
	mat3 normal_trans_matrix;
	};*/


//uniform mat4 camera_to_clip_matrix;
//uniform mat4 model_to_camera_matrix;
//uniform mat3 normal_trans_matrix;
//uniform mat4 view_matrix;

void main()
{
  //camera_pos =  model_to_camera_matrix * position;
	//camera_normal =  normal_trans_matrix*normal;
	//mat3 temp = transpose(inverse(mat3(model_to_camera_matrix)));
	//camera_normal = temp*normal;
	//camera_normal = abs(vec3(normal_trans_matrix[1]));
	//camera_normal = normal; //THIS IS FOR DEBUGGING!!!
  //camera_normal = normal_trans_matrix*normal;
  world_position =model_matrix*position;
  world_normal = normal_matrix*normal;
  //gl_Position = camera_to_clip_matrix * model_to_camera_matrix * position;
  //tex_coord_out = tex_coord;
  gl_Position = projection_matrix*(model_matrix*position);
  //gl_Position = position;
}
