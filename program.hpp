#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <GL/glew.h>
#include <string>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>

#define LOCAL_FILE_DIR "shaders/"
#define GLOBAL_FILE_DIR "./"

class Program {
private:

protected:

  //DISALLOW_COPY_AND_ASSIGN(Program);
  GLuint load_shader(GLenum shader_type, const std::string &shader_file_name);
  GLuint create_shader(GLenum shader_type,std::ifstream &shader_file_stream);
  
  GLuint program_ident;
  //GLuint ubo_index;
  
	      
public:
  Program(const std::string &vertex_shader_file, const std::string &fragment_shader_file,const std::string &geometry_shader_file = "");
  Program() {}
  GLuint get_program_ident() {
    return program_ident;
  }
  /*GLuint get_ubo_index() const {
    return ubo_index;
    }*/
  /*
  void set_ubo_index(std::string ubo_name,GLuint binding_point) {
    this->ubo_index  = glGetUniformBlockIndex(get_program(),ubo_name.c_str());
    assert(this->ubo_index != GL_INVALID_INDEX);
    //Last parameter should be global_ubo_binding_point
    glUniformBlockBinding(get_program(),get_ubo_index(),binding_point);
  }
  */
  void set_uniform_block_binding(std::string ubo_name,GLuint binding_point);
  void list_uniforms();
};
#endif
