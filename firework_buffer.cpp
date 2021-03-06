#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

#include "firework_buffer.hpp"

FireworkBuffer::FireworkBuffer() : Buffer("firework_buffer",buffer_length), creation_time(0), explosion_time(5) {
  
}

void FireworkBuffer::update_buffer_phase2(Object * parent) {
  glBindBuffer(GL_UNIFORM_BUFFER,get_ubo_pointer());
  glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(glm::vec3),glm::value_ptr(initial_velocity));
  glBufferSubData(GL_UNIFORM_BUFFER,sizeof(glm::vec3),sizeof(creation_time),&creation_time);
  glBufferSubData(GL_UNIFORM_BUFFER,sizeof(glm::vec3)+sizeof(creation_time),sizeof(explosion_time),&explosion_time);
  glBindBuffer(GL_UNIFORM_BUFFER,0);
}
