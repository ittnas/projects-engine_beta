#ifndef VIEW_BUFFER_HPP
#define VIEW_BUFFER_HPP

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "buffer.hpp"


class ViewBuffer : public Buffer {
private:
  static const GLuint buffer_length = 4*16; //sizeof(GLfloat)*length(mat4)
  glm::vec3 forward_dir; // If camera orientation follows the motion of the object, update this using the model_matrix of dynamic_cast<Movable *>(Object *)
  glm::vec3 up_dir; // This too. opengl.org/discussion_boards/showthread.php/175515-Get-Direction-from-Transformation-Matrix-or-Quat offers some nice tips for getting the them from model_matrix.
  glm::fquat forward;
  glm::fquat up;
  glm::mat4 view_matrix;
  //GLboolean active;

  glm::mat4 construct_view_matrix(glm::vec4 position);
public:
  ViewBuffer();
  virtual ~ViewBuffer() {}
  void update_buffer(glm::vec4 position);
  virtual void update_buffer_phase1(Object * parent);
  glm::vec3 get_up_dir() {return up_dir;}
  glm::vec3 get_forward_dir() {return forward_dir;}
  glm::fquat get_forward() {return forward;}
  glm::fquat get_up() {return up;}
  void set_forward(glm::fquat forward) {this->forward = forward;}
  void set_up(glm::fquat up) {this->up = up;}
  glm::mat4 get_view_matrix() const;
  void update_view_matrix(glm::vec4 position);
  //void activate() {this->active = GL_TRUE;}
  //void deactivate() {this->active = GL_FALSE;}
};

#endif
