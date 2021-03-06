#ifndef LIGHTNING_BUFFER_HPP
#define LIGHTNING_BUFFER_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "buffer.hpp"
#include "gl_constants.hpp"

class LightningBuffer : public Buffer {
private:
  static const GLuint buffer_length = MAX_NBR_LIGHTS*(16+12+16+4+4+4+4+4+1+15); // +15 for padding
  
  glm::vec4 intensity;
  GLfloat exponent;
  GLfloat cutoff;
  GLfloat kc;
  GLfloat kl;
  GLfloat kq;
  GLuint light_index;
  GLboolean is_active;

public:
  LightningBuffer();
  virtual ~LightningBuffer() {}

  virtual void update_buffer_phase1(Object * parent);
  
  glm::vec4 get_intensity() const {return intensity;}
  GLfloat get_exponent() const {return exponent;}
  GLfloat get_cutoff() const {return cutoff;}
  GLfloat get_kc() const {return kc;}
  GLfloat get_kl() const {return kl;}
  GLfloat get_kq() const {return kq;}

  void set_intensity(glm::vec4 intensity) {
    this->intensity = intensity;
  }
  void set_exponent(GLfloat exponent) {
    this->exponent = exponent;
  }
  void set_cutoff(GLfloat cutoff) {
    this->cutoff = cutoff;
  }
  void set_kc(GLfloat kc) {
    this->kc = kc;
  }
  void set_kl(GLfloat kl) {
    this->kl = kl;
  }
  void set_kq(GLfloat kq) {
    this->kq = kq;
  }
  void set_light_index(GLuint index) {
    this->light_index = index;
  }
  void disable() {is_active = false;}
  void enable() {is_active = true;}

};

#endif
