#ifndef TEXTURE2D_HPP
#define TEXTURE2D_HPP

#include "texture.hpp"

class Texture2D : public Texture {
private:
  GLsizei width,height;
  GLubyte * data;
protected:
  GLsizei get_data_length_in_bytes();
public:
  Texture2D(GLubyte * const data, GLint internal_format, GLenum format, GLenum type, GLsizei width, GLsizei height);
  virtual ~Texture2D() {delete data;};
  virtual void print_texture();
};

#endif
