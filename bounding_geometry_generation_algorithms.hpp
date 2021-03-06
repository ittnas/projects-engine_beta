#ifndef BOUNDING_GEOMETRY_GENERATION_ALGORITHMS_HPP
#define BOUNDING_GEOMETRY_GENERATION_ALGORITHMS_HPP

#include "bounding_geometry.hpp"

namespace BGAlgorithm {

  BoundingSphere * ritters_algorithm(const std::vector<glm::vec4> & points);
  BoundingAABB * boundingAABB_from_points(const std::vector<glm::vec4> & points);
  BoundingOBB * simple_bounding_OBB(const std::vector<glm::vec4> & points);
}

#endif
