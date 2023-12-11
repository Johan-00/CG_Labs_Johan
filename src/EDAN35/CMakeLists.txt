add_library (parametric_shapes2 STATIC)
target_sources (
       parametric_shapes
       PUBLIC [[parametric_shapes.hpp]]
       PRIVATE [[parametric_shapes.cpp]]
)
target_link_libraries (parametric_shapes2 PRIVATE bonobo CG_Labs_options)


add_executable (EDAN35_Assignment2)
target_sources (
	EDAN35_Assignment2
	PRIVATE
		[[assignment2.hpp]]
		[[assignment2.cpp]]
 "assignment2.cpp")

target_link_libraries (EDAN35_Assignment2 PRIVATE assignment_setup)

copy_dlls (EDAN35_Assignment2 "${CMAKE_CURRENT_BINARY_DIR}")

add_executable (EDAN35_project)
target_sources (
	EDAN35_project
	PRIVATE
		[[project.hpp]]
		[[project.cpp]]
 "project.cpp")

target_link_libraries (EDAN35_project PRIVATE assignment_setup parametric_shapes)

copy_dlls (EDAN35_project "${CMAKE_CURRENT_BINARY_DIR}")

install (
	TARGETS
		EDAN35_Assignment2
		EDAN35_project
	DESTINATION [[bin]]
)
