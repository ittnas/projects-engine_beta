#ifndef MESH_ALGORITHMS_HPP
#define MESH_ALGORITHMS_HPP

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <unordered_set>
#include <queue>

namespace MeshAlgorithms {
  enum Algorithm {BOWYERWATSON,QUICKHULL,JARVISMARCH};
  class Sphere {
  private:
    glm::vec3 center;
    GLfloat radius;
  public:
    Sphere(glm::vec3 center,GLfloat radius);
    ~Sphere() {}
    glm::vec3 get_center() const {return center;}
    GLfloat get_radius() const {return radius;}
  };
  class Edge {
    
  };
  class Face {
  private:
    GLint p1;
    GLint p2;
    GLint p3;
    const std::vector<glm::vec4> & points;
    std::unordered_set<Face *> neighbours;
    glm::vec3 normal_direction; // This is not normalized!
  public:
    Face(GLint p1,GLint p2,GLint p3,const std::vector<glm::vec4> & points);
    Face(GLint p1,GLint p2,GLint p3,const std::vector<glm::vec4> & points,glm::vec3 normal);
    ~Face() {}
    void swap_orientation();
    glm::vec3 get_normal() {return normal_direction;}
    glm::vec4 get_v1() const {return points[p1];}
    glm::vec4 get_v2() const {return points[p2];}
    glm::vec4 get_v3() const {return points[p3];}
    GLint get_p1() const {return p1;}
    GLint get_p2() const {return p2;}
    GLint get_p3() const {return p3;}
    GLint get_vertex(GLint number) const;
    void add_neighbour(Face * face) {neighbours.insert(face);}
    void remove_neighbour(Face * face) {neighbours.erase(face);}
    std::unordered_set<Face *> * get_neighbours() {return &neighbours;}
  };
  class Tetrahedron {
  private:
    glm::mat4 vertices;
  public:
    glm::mat4 get_vertices() const {return vertices;}
    Tetrahedron(glm::vec4 vert1,glm::vec4 vert2, glm::vec4 vert3, glm::vec4 vert4);
    ~Tetrahedron() {}
  };

  Sphere * create_circumsphere(const Tetrahedron & tetr);
  void delanay_triangulation(const std::vector<glm::vec4> & points,std::vector<Tetrahedron *> & triangulation,Algorithm algorithm = BOWYERWATSON);
  GLboolean tetrahedra_have_shared_edge(const Tetrahedron & tetr1,const Tetrahedron & tetr2);
  Tetrahedron * create_super_tetrahedron(const std::vector<glm::vec4>& points);
  void bowyer_watson(const std::vector<glm::vec4> & points, std::vector<Tetrahedron *> & triangulation);
  void create_convex_hull_3d(const std::vector<glm::vec4> & points,std::vector<GLint> & convex_hull,Algorithm algorithm = QUICKHULL);
  void create_convex_hull_2d(const std::vector<glm::vec2> & points,std::vector<GLint> & convex_hull,Algorithm = JARVISMARCH);
  void quickhull(const std::vector<glm::vec4> & points,std::vector<GLint> & convex_hull);
  void find_extremum_points(const std::vector<glm::vec4> & points,std::vector<GLint> & extrema);
  GLint farthest_point_to_line(const std::vector<glm::vec4> & points,glm::vec4 line_vert1,glm::vec4 line_vert2);
  GLint farthest_point_to_triangle(const std::vector<glm::vec4> & points,const glm::vec3 & vert1,const glm::vec3 & vert2, const glm::vec3 & vert3);
  GLfloat distance2_to_triangle(const glm::vec4 & P, const glm::vec3 & vert1,const glm::vec3 & vert2, const glm::vec3 & vert3, glm::vec3 & point_on_triangle);
  void create_orthogonal_basis(const glm::vec3 & v1, glm::vec3 & v2,glm::vec3 & v3);
  void find_light_faces(std::queue<Face *> & light_faces,std::unordered_set<Face *> & visited_faces,Face * face,GLint farthest_point,const std::vector<glm::vec4> & points);
  void quickhull(const std::vector<glm::vec2> & points,std::vector<GLint> & convex_hull);
  void jarvis_march(const std::vector<glm::vec2> & points,std::vector<GLint> & convex_hull);
  GLboolean left_of_line(const glm::vec2 & p,const glm::vec2 & v1,const glm::vec2 & v2);
}


#endif
