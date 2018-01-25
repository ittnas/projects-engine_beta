#include "bounding_geometry_factory.hpp"

BoundingGeometryFactory::BoundingGeometryFactory() {
  //TODO
  // Set all the default algorithms
}

void BoundingGeometryFactory::set_algorithm_bounding_sphere_from_bounding_box(BoundingSphereFromBoxAlgorithm bsfba) {
  this->bsfba = bsfba;
}

void BoundingGeometryFactory::set_algorithm_bounding_sphere_from_bounding_sphere(BoundingSphereFromSphereAlgorithm bsfsa) {
  this->bsfsa = bsfsa;
}

/*
  BoundingGeometry * BoundingGeometryFactory::create_bounding_geometry(BoundingGeometryType bgt) {
  switch(bgt) {
  case SPHERE:
  return 
  
  }
  }
*/

