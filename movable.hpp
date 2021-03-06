#ifndef MOVABLE_HPP
#define MOVABLE_HPP

#include "object.hpp"
#include "model_buffer.hpp"

class Movable : virtual public Object {
private:
  ModelBuffer * model_buffer;
public:
  Movable();
  virtual ~Movable() {}
  glm::vec4 get_position() const {return model_buffer->get_position();}
  glm::vec4 get_position_global() const {return model_buffer->get_position_global();}
  glm::fquat get_orientation() const {return model_buffer->get_orientation();}
  glm::vec3 get_scale() const {return model_buffer->get_scale();}
  GLfloat get_uniform_scale() {return model_buffer->get_uniform_scale();}
  glm::mat4 get_model_matrix() const {return model_buffer->get_model_matrix();}
  
  void set_position(glm::vec4 position) {model_buffer->set_position(position);}
  void set_orientation(glm::fquat orientation) {model_buffer->set_orientation(orientation);}
  void set_scale(glm::vec3 scale) {model_buffer->set_scale(scale);}
  void set_scale(GLdouble scale) {model_buffer->set_scale(scale);}
  void set_center_of_mass(const glm::vec3 & com);
  glm::vec3 get_center_of_mass() const;
  void get_transformed_mesh_points(std::vector<glm::vec4> & output, GLuint attribute_array_index = 0,GLuint physics_flag = 0xFFFF,Comparator = ANY); // physics_flag should be actually 0xFFFF
};

#endif

