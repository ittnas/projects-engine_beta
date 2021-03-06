#include "rectangle.hpp"

Rectangle::Rectangle(GLfloat width,GLfloat height) : Shape(), width(width),height(height) {
  set_mesh();
}

Mesh * Rectangle::create_mesh() {
  Mesh * mesh = new Mesh(TRIANGLE,INDEXED);
  GLfloat data[4*4] = {
    0,0,0,1,
    0,height,0,1,
    width,height,0,1,
    width,0,0,1
  };
  GLfloat normal_data[4*3] = {
    0,0,1,
    0,0,1,
    0,0,1,
    0,0,1
  };
 
  GLfloat tex_coord[4*2] = {
    0,1,
    0,0,
    1,0,
    1,1
  };

  GLuint index_data[2*3] = {
    0,1,2,
    0,2,3
  };
  mesh->add_attribute_array(4,4,data);
  mesh->add_attribute_array(3,4,normal_data);
  mesh->add_attribute_array(2,4,tex_coord);
  mesh->set_index_array(6,index_data);
  //mesh->convert_to_triangles();
  mesh->create_adjacency_data();
  
  mesh->save_to_memory();
  mesh->save_index_data_to_memory();
  mesh->save_adjacency_data_to_memory(GL_FALSE);
  return mesh;
}
