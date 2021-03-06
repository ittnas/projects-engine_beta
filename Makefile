CC = g++
CFLAGS = -g -std=c++11 -I/usr/include/freetype2
LIBS = -lSDL2 -lGL -lGLU -lGLEW -lpng -lfreetype -lpthread
OBJ = buffer.o ubo.o main.o model_buffer.o drawer.o action.o object.o program.o shape.o mesh.o attribute_array.o index_array.o matrix_stack.o world.o projection_buffer.o movable.o camera.o view_buffer.o tetrahedron.o viewer.o drawer.o key_table.o flying_camera.o key_action.o light_manager.o lightning_buffer.cpp light.o uniform_material.cpp uniform_material_buffer.o buffer_list.o parametric_plot.o global_context.o update_time_action.o firework_shape.o firework_shooter.o firework_buffer.o firework.o firework_action.o killer_action.o text_object.o opengl_data_type_enum_length.o texture.o texture1D.o texture_buffer.o texture2D_object.o texture2D.o uniform_list.o texture_uniform_container.o oscillator_action.o character_object.o rectangle.o rendering_function.o physics_manager.o simple_collision_detector.o aabb.o cube.o rigid_body.o bounding_geometry.o collision_object.o bounding_geometry_generation_algorithms.o parametric_functions.o uniform_color_collision_object.o parametric_surface_generator.o collision_hierarchy.o collision_object.o collision_hierarchy_node.o collision_pair.o collision_detector.o simple_collision_hierarchy.o collision_callback_function.o collision_response_algorithms.o collision_detection_algorithms.o cuboid.o force.o physics_object.o triangle.o basic_geometry_shapes.o mesh_algorithms.o debug_utility_functions.o fps_logger.o fps_text_object.o indexed_shape.o

main: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

#%.o: %.cpp $(DEPS)
#	$(CC) $(CFLAGS) -c -o $@ $<

#%.o: %.cpp %.hpp
#	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CC) $(CFLAGS) -MD -c $(LIBS) -o $@ $<
	@cp $*.d $*.P;\
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P;\
	rm -f $*.d
#%.o: %.ccp
#	$(CC) $(CFLAGS) -MD -c $(LIBS) -o $@ $<
#	@cp $*.d $*.P; \
sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
rm -f $*.d

-include *.P
