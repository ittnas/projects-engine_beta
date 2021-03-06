#include "opengl_data_type_enum_length.hpp"

std::unordered_map<GLenum,GLsizei> DataType::data_types = {{GL_FLOAT,sizeof(GLfloat)},{GL_UNSIGNED_INT,sizeof(GLuint)},{GL_UNSIGNED_BYTE,sizeof(GLubyte)}};

GLsizei DataType::sizeof_enum(GLenum data_type) {
  return data_types.at(data_type);
}

std::unordered_map<GLenum,GLsizei> PixelType::pixel_types = {{GL_R,1},{GL_RGBA,4},{GL_ALPHA,1}};

GLsizei PixelType::nbr_elements_per_pixel(GLenum pixel_type) {
  return pixel_types.at(pixel_type);
}
