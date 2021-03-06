#include "texture2D_object.hpp"
#include "texture2D.hpp"

Texture2DObject::Texture2DObject() : Object(), Movable(),UniformMaterial() {
  this->texture_uniform_container = new TextureUniformContainer();
  if(this->texture_uniform_container == 0) {
    assert(0); // Out of memory! :(
  }
  this->add_uniform_container(this->texture_uniform_container);
  //TODO create the texture here.
  GLsizei height = 4;
  GLsizei width = 4;
  GLubyte data[height*width*4*sizeof(GLubyte)];
  create_texture(data,height,width);
  Texture2D * texture = new Texture2D(data,GL_RGBA,GL_RGBA,GL_UNSIGNED_BYTE,width,height);
  this->texture_uniform_container->add_texture("diffusive_color_sampler",texture,2);
}

void Texture2DObject::create_texture(GLubyte * texture,GLsizei height, GLsizei width) {
  for(GLsizei i = 0; i < height;i++) {
    for(GLsizei j = 0; j < width;j++) {
      texture[4*(i*width + j)] = 128;
      texture[4*(i*width + j)+1] = 0;
      texture[4*(i*width + j)+2] = 128;
      texture[4*(i*width + j)+3] = 255;
    }
  }
}
