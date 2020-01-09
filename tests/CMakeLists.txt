add_executable(run_test
    tests.cpp
    000_triangle.cpp
	001_spinning_cube.cpp
	002_mesh_viewer.cpp
)

target_link_libraries(run_test
	glad
	SDL2 SDL2main
	glm
	imgui
	stb
	tl
	tgl
	cgltf
)
