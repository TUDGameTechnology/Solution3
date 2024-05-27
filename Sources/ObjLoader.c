#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <kinc/io/filereader.h>

#include "ObjLoader.h"
#include "Memory.h"

#define MAX_TOKEN_LENGTH 256

void tokenize(char* s, char delimiter, int* i, char* token) {
	int last_index = *i;
	char* index = strchr(s + last_index + 1, delimiter);
	if (index == NULL) {
		token[0] = 0;
		return;
	}
	int new_index = (int)(index - s);
	*i = new_index;
	int length = new_index - last_index;
	assert(length < MAX_TOKEN_LENGTH);
	strncpy(token, s + last_index + 1, length);
	token[length] = 0;
}

int count_first_char_lines(char* source, const char* start) {
	int count = 0;

	int index = 0;
	char line[MAX_TOKEN_LENGTH];
	tokenize(source, '\n', &index, line);

	while (line[0] != 0) {
		char *pch = strstr(line, start);
		if (pch == line)
			count++;
		tokenize(source, '\n', &index, line);
	}
	return count;
}

int count_faces_in_line(char* line) {
	char* token = strtok(line, " ");
	int i = -1;
	while (token != NULL) {
		token = strtok(NULL, " ");
		i++;
	}

	// For now, handle tris and quads

	if (i == 3) {
		return 1;
	}
	else {
		return 2;
	}
}

int count_faces(char* source) {
	int count = 0;

	int index = 0;
	char line[MAX_TOKEN_LENGTH];
	tokenize(source, '\n', &index, line);

	while (line[0] != 0) {
		if (line[0] == 'f') {
			count += count_faces_in_line(line);
		}
		tokenize(source, '\n', &index, line);
	}
	return count;
}

int count_vertices(char* source) {
	return count_first_char_lines(source, "v ");
}

int count_uvs(char* source) {
	return count_first_char_lines(source, "vt ");
}

void parse_vertex(struct Mesh* mesh, char* line) {
	char* token;
	for (int i = 0; i < 3; i++) {
		token = strtok(NULL, " ");
		mesh->cur_vertex[i] = (float)strtod(token, NULL);
	}

	mesh->cur_vertex += 3;
	mesh->cur_vertex[0] = 0;
	mesh->cur_vertex[1] = 0;
	mesh->cur_vertex += 2;

	mesh->num_vertices++;
}

void set_uv(struct Mesh* mesh, int index, float u, float v) {
	mesh->vertices[(index * 5) + 3] = u;
	mesh->vertices[(index * 5) + 4] = v;
}

void parse_face(struct Mesh* mesh, char* line) {
	char* token;
	int verts[4];
	int uv_index[4];
	bool has_uv[4];
	for (int i = 0; i < 3; i++) {
		token = strtok(NULL, " ");
		char* endPtr;
		verts[i] = (int)strtol(token, &endPtr, 0) - 1;
		if (endPtr[0] == '/') {
			// Parse the uv
			has_uv[i] = true;
			uv_index[i] = (int)strtol(endPtr + 1, NULL, 0) - 1;
		}
		else {
			// There is no uv
			has_uv[i] = false;
		}
	}
	token = strtok(NULL, " ");
	//char* result;
	if (token != NULL) {
		verts[3] = (int)strtol(token, NULL, 0) - 1;
		// We have a quad
		mesh->cur_index[0] = verts[0];
		mesh->cur_index[1] = verts[1];
		mesh->cur_index[2] = verts[2];
		mesh->cur_index += 3;
		mesh->cur_index[0] = verts[2];
		mesh->cur_index[1] = verts[3];
		mesh->cur_index[2] = verts[0];
		mesh->cur_index += 3;
		mesh->num_faces += 2;
	}
	else {
		// We have a triangle
		for (int i = 0; i < 3; i++) {
			mesh->cur_index[i] = verts[i];

			if (!has_uv[i]) continue;

			// Set the UVs
			set_uv(mesh, mesh->cur_index[i], mesh->uvs[uv_index[i] * 2], mesh->uvs[(uv_index[i] * 2) + 1]);
		}
		mesh->cur_index += 3;
		mesh->num_faces += 1;
	}
}

void parse_uv(struct Mesh* mesh, char* line) {
	char* token;
	for (int i = 0; i < 2; i++) {
		token = strtok(NULL, " ");
		*mesh->cur_uv = (float)strtod(token, NULL);
		mesh->cur_uv++;
	}
}

void parse_line(struct Mesh* mesh, char* line) {
	char* token = strtok(line, " ");
	if (strcmp(token, "v") == 0) {
		// Read some vertex data
		parse_vertex(mesh, line);
	}
	else if (strcmp(token, "f") == 0) {
		// Read some face data
		parse_face(mesh, line);
	}
	else if (strcmp(token, "vt") == 0) {
		parse_uv(mesh, line);
	}

	// Ignore all other commands (for now)	
}

struct Mesh* load_obj(const char* filename) {
	struct kinc_file_reader file_reader;
	kinc_file_reader_open(&file_reader, filename, KINC_FILE_TYPE_ASSET);
	size_t file_size = kinc_file_reader_size(&file_reader);
	char* source = memory_scratch_pad(sizeof(char) * (file_size + 1));
	kinc_file_reader_read(&file_reader, source, file_size);
	source[file_size] = 0;

	struct Mesh* mesh = memory_allocate(sizeof(struct Mesh));

	int vertices = count_vertices(source);
	mesh->vertices = memory_allocate(sizeof(float) * vertices * 5);
	mesh->cur_vertex = mesh->vertices;
	int faces = count_faces(source);
	mesh->indices = memory_allocate(sizeof(int) * faces * 3);
	mesh->cur_index = mesh->indices;
	mesh->num_uvs = count_uvs(source);
	mesh->uvs = memory_allocate(sizeof(float) * mesh->num_uvs * 2);
	mesh->cur_uv = mesh->uvs;
	
	mesh->num_vertices = 0;
	mesh->num_faces = 0;
	
	int index = 0;
	char line[MAX_TOKEN_LENGTH];
	tokenize(source, '\n', &index, line);
	
	while (line[0] != 0) {
		parse_line(mesh, line);
		tokenize(source, '\n', &index, line);
	}

	return mesh;
}
