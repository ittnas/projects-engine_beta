#include <random>
#include <iostream>
#include <cmath>

#include "firework_shape.hpp"

FireworkShape::FireworkShape(GLuint n, GLfloat mean_velocity,GLfloat stdev_velocity,glm::vec3 color,glm::vec3 color_stdev,GLuint seed) : n(n), mean_velocity(mean_velocity), stdev_velocity(stdev_velocity), seed(seed), color(color), color_stdev(color_stdev) {
  set_mesh();
}

Mesh * FireworkShape::create_mesh() {
  Mesh * mesh = new Mesh(POINTS,ARRAY);
  if(seed != -1) {
    srand(seed);
  }
  GLfloat * velocity_data = new GLfloat[n*3];
  GLfloat * pointsize_data = new GLfloat[n];
  GLfloat * color_data = new GLfloat[n*3];
  //get_uniform_random_nbr( min_velocity, GLfloat max_velocity,GLuint n,GLfloat 
  /*
  for(GLuint i = 0; i < n*3; i++) {
    velocity_data[i] = get_random_nbr(min_velocity,max_velocity);
  }
  */
  get_gaussian_random_nbr_3d(mean_velocity,stdev_velocity,n,velocity_data);
  get_gaussian_random_nbr(2.5,2,n,pointsize_data);
  
  get_gaussian_random_nbr(color.x,color_stdev.x,n,color_data,3);
  get_gaussian_random_nbr(color.y,color_stdev.y,n,color_data+1,3);
  get_gaussian_random_nbr(color.z,color_stdev.z,n,color_data+2,3);

  /*for(GLuint i = 0; i < n; i++) {
    std::cout << color_data[i*3+0] << ":" << color_data[i*3+1] << ":" << color_data[i*3+2] << std::endl;
    }*/
    /*
  for(GLuint i = 0; i < n; i++) {
    pointsize_data[i] = get_random_nbr(1,5);
  }
    */
  mesh->add_attribute_array(3,n,velocity_data);
  mesh->add_attribute_array(1,n,pointsize_data);
  mesh->add_attribute_array(3,n,color_data);
  mesh->save_to_memory();
  delete velocity_data;
  delete pointsize_data;
  delete color_data;
  return mesh;
}

GLfloat FireworkShape::get_random_nbr(GLfloat min, GLfloat max) {
  GLfloat rnd = min + (max-min)*(rand()/(GLfloat)((RAND_MAX)));
  return rnd;
}

void FireworkShape::get_gaussian_random_nbr(GLfloat mean, GLfloat stdev,GLuint n,GLfloat * output,GLuint offset) {
  std::random_device rd;
  std::mt19937 e2(rd());
  std::normal_distribution<> dist(mean,stdev);
  for(GLuint i = 0; i < n;i++) {
    output[i*offset] = dist(e2);
  }
}

void FireworkShape::get_gaussian_random_nbr_3d(GLfloat mean, GLfloat stdev,GLuint n,GLfloat * output) {
  GLfloat * u = new GLfloat[n];
  GLfloat * phi = new GLfloat[n];
  GLfloat * r = new GLfloat[n];

  get_uniform_random_nbr(-1,1,n,u);
  get_uniform_random_nbr(0,2*3.1415925,n,phi);
  get_gaussian_random_nbr(mean,stdev,n,r);

  for(GLuint i = 0; i < n; i++) {
    output[i*3] = r[i]*sqrt(1-u[i]*u[i])*cos(phi[i]);
    output[i*3+1] = r[i]*sqrt(1-u[i]*u[i])*sin(phi[i]);
    output[i*3+2] = r[i]*u[i];
  }
  delete u;
  delete phi;
  delete r;
}


void FireworkShape::get_uniform_random_nbr(GLfloat lower, GLfloat upper, GLuint n, GLfloat * output) {
  std::random_device rd;
  std::mt19937 e2(rd());
  std::uniform_real_distribution<> dist(lower,upper);
  for(GLuint i = 0; i < n; i++) {
    output[i] = dist(e2);
  }
}
