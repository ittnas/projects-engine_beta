#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

#include "projection_buffer.hpp"
#include "global_context.hpp"

//MatrixStack * ProjectionBuffer::projection_stack = new MatrixStack();
//MatrixStack * ProjectionBuffer::projection_stack = GlobalContext::projection_stack;

void ProjectionBuffer::update_buffer_phase1(Object * parent) {
  glBindBuffer(GL_UNIFORM_BUFFER,get_ubo_pointer());
  glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(glm::mat4),glm::value_ptr(projection_matrix));
  glBufferSubData(GL_UNIFORM_BUFFER,sizeof(glm::mat4),sizeof(tick),&tick);
  glBindBuffer(GL_UNIFORM_BUFFER,0);
}

void ProjectionBuffer::set_projection(glm::mat4 projection_matrix) {
  this->projection_matrix = projection_matrix;
  // This does not update the UBO. It should be done in the update_buffer_phase1.
}

glm::mat4 ProjectionBuffer::create_perspective_projection_matrix(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearval, GLfloat farval,GLfloat inf) {
      GLfloat x,y,a,b,c,d;
  x = (2.0*nearval) / (right - left);
  y = (2.0*nearval) / (top - bottom);
  a = (right + left) / ( right - left);
  b = (top + bottom) / (top - bottom);

  if (farval >= inf) {
    c = -1.0;
    d = -2.0 * nearval;
  } else {
    c = -(farval + nearval) / (farval - nearval);
    d = -(2.0 * farval * nearval)/(farval - nearval);
  }
  glm::mat4 output(0.0);

  output[0].x = x;
  output[1].y = y;
  output[2].z = c;
  output[2].w = -1.0f;
  output[3].z = d;
  return output;
}

ProjectionBuffer::ProjectionBuffer() : Buffer("projection_buffer",buffer_length),tick(0) {
  projection_stack = GlobalContext::projection_stack;
}

ProjectionBuffer::~ProjectionBuffer() {
}
