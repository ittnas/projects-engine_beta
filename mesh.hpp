#ifndef MESH_HPP
#define MESH_HPP

#include<vector>
#include<glm/glm.hpp>

#include "attribute_array.hpp"
#include "index_array.hpp"
#include "polygon_type.hpp"

enum DrawingMode{
  ARRAY,INDEXED,ADJACENCY,NOTSET
};

#define DUMMY

class Mesh {

private:
  std::vector<AttributeArray *> attribute_arrays;
  IndexArray * index_array;
  GLuint * adjacency_array;
  
  GLuint buffer_handle;
  GLuint index_buffer_handle;
  //  GLuint adjacency_buffer_handle;
  GLuint vao_handle;
  DrawingMode drawing_mode;
  PolygonType polygon_type;
  GLboolean interleaved; //TODO, not used yet.
  // Some secondary properties, used to save time
  //std::unordered_map<GLuint,GLfloat> volumes;
  glm::mat3 get_covariance_matrix(GLuint attribute_array_index);
public:
  /** 
   * Adds a new attribute array with the largest index.
   * 
   * @param array 
   */
  void add_attribute_array(AttributeArray * array) {
    attribute_arrays.push_back(array);
  }
  AttributeArray * get_attribute_array(GLuint index) {
    return attribute_arrays[index];
  }

  /** 
   * Adds attribute array with given index. If index is larger than the largest previous index, the index of the added array will be ind_array = prev_largest_ind + 1.
   * 
   * @param array 
   * @param index 
   */
  void insert_attribute_array(AttributeArray * array, GLuint index) {
    attribute_arrays.insert(attribute_arrays.begin() + index,array);
  }

  void add_attribute_array(GLuint nbr_elements_per_vertex, GLuint nbr_vertices, GLfloat * data, GLenum normalize = GL_FALSE) {
    add_attribute_array(new AttributeArray(nbr_elements_per_vertex,nbr_vertices,data,normalize));
  }

  GLuint append_data(GLuint nbr_vertices, GLfloat * data, GLuint index);

  void insert_attribute_array(GLuint index, GLuint nbr_elements_per_vertex, GLuint nbr_vertices, GLfloat * data, GLenum normalize = GL_FALSE) {
    insert_attribute_array(new AttributeArray(nbr_elements_per_vertex,nbr_vertices,data,normalize), index);
  }

  void set_polygon_type(PolygonType ptype) {
    this->polygon_type = ptype;
  }

  void save_to_memory(GLboolean interleaved = GL_TRUE);
  void save_index_data_to_memory();
  void save_adjacency_data_to_memory(GLboolean override_index_buffer = 0);
  GLfloat * create_buffer_data(GLboolean interleaved);
  GLuint calculate_buffer_length();

  // There is a problem here. This version does not create a new array, on the other hand the one with nbr_vertices does...
  void set_index_array(IndexArray * index_array) {
    /*if(this->index_array) {
      delete this->index_array;
      }*/
    this->index_array = index_array;}
  
  void set_index_array(GLuint nbr_vertices,GLuint * data) {
    // There is a memory problem present: The user is responsible for destroying the array. However, convert_to_triangles() creates a new array but does not free it.
    /*if(this->index_array) {
      delete this->index_array;
      }*/
    index_array = new IndexArray(nbr_vertices,data);
  }
  void clear_index_array() {delete index_array; index_array = 0;}

  void draw(DrawingMode mode = NOTSET);

  void set_drawing_mode(DrawingMode drawing_mode);
  Mesh(PolygonType polygon_type = TRIANGLE,DrawingMode drawing_mode = ARRAY);
  ~Mesh();

  void print_contex(const GLfloat * input, GLuint nbr_elements) const;
  GLint convert_to_triangles();
  GLuint create_adjacency_data();
  void insert_adjacency_array(std::vector<GLuint> & adjacency_data);
  GLboolean displace_mesh(GLuint attribute_array_index,GLuint elements,const GLfloat * displacement);
  GLboolean rotate_mesh(GLuint attribute_array_index,const glm::mat3 & rotation_matrix);
  void get_center_of_mass(GLuint attribute_array_index,glm::vec3 & center_of_mass,GLfloat & mass);
  void get_center_of_mass(GLuint attribute_array_index,GLuint density_array_index,glm::vec3 & center_of_mass,GLfloat & mass);
  glm::mat3 get_moment_of_inertia(GLuint attribute_array_index);//, const glm::vec3 & rp = glm::vec3(0,0,0));
  //GLfloat get_volume(GLuint attribute_index_array);
  void get_points(std::vector<glm::vec4> & output, GLuint attribute_array_index);
  void get_random_points_inside_mesh(GLuint nbr_points,GLuint target_attribute_array,std::vector<glm::vec4> & output,GLuint seed,GLboolean is_convex);
  GLboolean get_index_data(std::vector<GLuint> & index_data);
};

#endif


