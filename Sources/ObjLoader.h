#pragma once

struct Mesh {
	int num_faces;
	int num_vertices;
	int num_uvs;

	float* vertices;
	int* indices;
	float* uvs;

	// very private
	float* cur_vertex;
	int* cur_index;
	float* cur_uv;
};

struct Mesh* load_obj(const char* filename);
