#include "collision_hierarchy_node.hpp"
#include "bounding_geometry_type.hpp"
#include "bounding_geometry_generation_algorithms.hpp"
#include "world.hpp"

CollisionHierarchyNode::CollisionHierarchyNode(CollisionObject * object) : collision_object(object) {
  
}

void CollisionHierarchyNode::generate_bounding_geometry(const std::vector<BoundingGeometry *> & source_geometries) {
  //TODO handle source_geometries
    BoundingGeometryType preferred_bounding_geometry = collision_object->get_preferred_bounding_volume();
    std::vector<glm::vec4> mesh;
    collision_object->get_transformed_mesh_points(mesh,0);
    //std::cout<<"Mesh size:"<<mesh.size()<<std::endl;
    //for (auto &i: mesh) {
    //  std::cout << glm::to_string(i) << std::endl;
    //}

    if(local_bounding_geometry != NULL) {
      delete local_bounding_geometry;
    }
    switch(preferred_bounding_geometry) {
    case SPHERE:
      local_bounding_geometry = BGAlgorithm::ritters_algorithm(mesh);
      break;
    case AABB:
      local_bounding_geometry = BGAlgorithm::boundingAABB_from_points(mesh);
      break;
    case OBB:
      local_bounding_geometry = BGAlgorithm::simple_bounding_OBB(mesh);
      break;
    default:
      assert(0);
      local_bounding_geometry = BGAlgorithm::ritters_algorithm(mesh);
    }
    //TODO handle composition from many bounding geometries
}

GLboolean CollisionHierarchyNode::create_bounding_geometry_object(Program * prog) {
  if(local_bounding_geometry != NULL) {
    bounding_geometry_object = new Movable();
    bounding_geometry_object->set_shape(local_bounding_geometry->create_shape());
    bounding_geometry_object->set_program(prog);
    Object * parent = collision_object->get_parent();
    if(parent != NULL) {
      parent->add_child(bounding_geometry_object);
    } else {
      World * collision_object_home_world = collision_object->get_world();
      if(collision_object_home_world == NULL) {
	assert(0); // This is actually an error
	return GL_FALSE;
      }
      collision_object_home_world->add_child(bounding_geometry_object);
    }
    return GL_TRUE;
  } else {
    return GL_FALSE;
  }
}

void CollisionHierarchyNode::update_bounding_geometry() {
  if(local_bounding_geometry != NULL) {
    local_bounding_geometry->transform_geometry(collision_object->get_position(),collision_object->get_orientation(),collision_object->get_scale().x,collision_object->get_center_of_mass());
  }
  if(bounding_geometry_object != NULL) {
    bounding_geometry_object->set_position(collision_object->get_position());
    bounding_geometry_object->set_orientation(collision_object->get_orientation());
    bounding_geometry_object->set_scale(collision_object->get_scale());
    bounding_geometry_object->set_center_of_mass(collision_object->get_center_of_mass());
    //bounding_geometry_object->transform(collision_object->get_position(),collision_object->get_orientation(),collision_object->get_scale());
  }
}

/*
void CollisionHierarchyNode::set_bounding_geometry_object_program(Program ** prog, GLuint program_position) {
  if(bounding_geometry_object_programs.size() <= program_position) {
    bounding_geometry_object_programs.resize(program_position+1,0);
  }
  bounding_geometry_object_programs.at(program_position) = prog;
}
*/
