#include <iostream>
#include <GL/glew.h>

#include "debug_utility_functions.hpp"

void DebugUtilityFunctions::print(glm::vec3 vec) {
  for(GLuint ii=0;ii<3;ii++) {
    std::cout << vec[ii] << ",\t";
  }
  std::cout << std::endl;
}

void DebugUtilityFunctions::print(glm::vec4 vec) {
  for(GLuint ii=0;ii<4;ii++) {
    std::cout << vec[ii] << ",\t";
  }
  std::cout << std::endl;
}

void DebugUtilityFunctions::print(glm::mat4 mat) {
  for(GLuint ii=0;ii<4;ii++) {
    for(GLuint jj=0;jj<4;jj++) {
      std::cout << mat[ii][jj] << ",\t ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void DebugUtilityFunctions::print(glm::mat3 mat) {
  for(GLuint ii=0;ii<3;ii++) {
    for(GLuint jj=0;jj<3;jj++) {
      std::cout << mat[ii][jj] << ",\t ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void DebugUtilityFunctions::print(std::string text) {
  #ifdef DEBUG
  std::cout << text;
  #endif
}
