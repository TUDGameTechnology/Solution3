#include "pch.h"
#include "ObjLoader.h"
#include "Memory.h"
#include <Kore/IO/FileReader.h>
#include <cstring>
#include <cstdlib>
#include <assert.h>

using namespace Kore;

namespace {
	const int maxTokenLength = 256;
	
	void tokenize(char* s, char delimiter, int& i, char* token) {
		int lastIndex = i;
		char* index = strchr(s + lastIndex + 1, delimiter);
		if (index == nullptr) {
			token[0] = 0;
			return;
		}
		int newIndex = (int)(index - s);
		i = newIndex;
		int length = newIndex - lastIndex;
		assert(length < maxTokenLength);
		strncpy(token, s + lastIndex + 1, length);
		token[length] = 0;
	}
	
	int countFirstCharLines(char* source, const char* start) {
		int count = 0;
		
		int index = 0;
		char line[maxTokenLength];
		tokenize(source, '\n', index, line);
		
		while (line[0] != 0) {
			char *pch = strstr(line, start);
			if (pch == line)
				count++;
			tokenize(source, '\n', index, line);
		}
		return count;
	}
	
	int countFacesInLine(char* line) {
		char* token = strtok(line, " ");
		int i = -1;
		while (token != nullptr) {
			token = strtok(nullptr, " ");
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
	
	int countFaces(char* source) {
		int count = 0;
		
		int index = 0;
		char line[maxTokenLength];
		tokenize(source, '\n', index, line);
		
		while (line[0] != 0) {
			if (line[0] == 'f') {
				count += countFacesInLine(line);
			}
			tokenize(source, '\n', index, line);
		}
		return count;
	}
	
	int countVertices(char* source) {
		return countFirstCharLines(source, "v ");
	}
	
	int countUVs(char* source) {
		return countFirstCharLines(source, "vt ");
	}
	
	void parseVertex(Mesh* mesh, char* line) {
		char* token;
		for (int i = 0; i < 3; i++) {
			token = strtok(nullptr, " ");
			mesh->curVertex[i] = (float)strtod(token, nullptr);
		}
		
		mesh->curVertex += 3;
		mesh->curVertex[0] = 0;
		mesh->curVertex[1] = 0;
		mesh->curVertex += 2;
		
		mesh->numVertices++;
	}
	
	void setUV(Mesh* mesh, int index, float u, float v) {
		mesh->vertices[(index * 5) + 3] = u;
		mesh->vertices[(index * 5) + 4] = v;
	}
	
	void parseFace(Mesh* mesh, char* line) {
		char* token;
		int verts[4];
		int uvIndex[4];
		bool hasUV[4];
		for (int i = 0; i < 3; i++) {
			token = strtok(nullptr, " ");
			char* endPtr;
			verts[i] = (int)strtol(token, &endPtr, 0) - 1;
			if (endPtr[0] == '/') {
				// Parse the uv
				hasUV[i] = true;
				uvIndex[i] = (int)strtol(endPtr + 1, nullptr, 0) - 1;
			}
			else {
				// There is no uv
				hasUV[i] = false;
			}
		}
		token = strtok(nullptr, " ");
		//char* result;
		if (token != nullptr) {
			verts[3] = (int)strtol(token, nullptr, 0) - 1;
			// We have a quad
			mesh->curIndex[0] = verts[0];
			mesh->curIndex[1] = verts[1];
			mesh->curIndex[2] = verts[2];
			mesh->curIndex += 3;
			mesh->curIndex[0] = verts[2];
			mesh->curIndex[1] = verts[3];
			mesh->curIndex[2] = verts[0];
			mesh->curIndex += 3;
			mesh->numFaces += 2;
		}
		else {
			// We have a triangle
			for (int i = 0; i < 3; i++) {
				mesh->curIndex[i] = verts[i];
				
				if (!hasUV[i]) continue;
				
				// Set the UVs
				setUV(mesh, mesh->curIndex[i], mesh->uvs[uvIndex[i] * 2], mesh->uvs[(uvIndex[i] * 2) + 1]);
			}
			mesh->curIndex += 3;
			mesh->numFaces += 1;
		}
	}
	
	void parseUV(Mesh* mesh, char* line) {
		char* token;
		for (int i = 0; i < 2; i++) {
			token = strtok(nullptr, " ");
			*mesh->curUV = (float)strtod(token, nullptr);
			mesh->curUV++;
		}
	}
	
	void parseLine(Mesh* mesh, char* line) {
		char* token = strtok(line, " ");
		if (strcmp(token, "v") == 0) {
			// Read some vertex data
			parseVertex(mesh, line);
		}
		else if (strcmp(token, "f") == 0) {
			// Read some face data
			parseFace(mesh, line);
		}
		else if (strcmp(token, "vt") == 0) {
			parseUV(mesh, line);
		}
		
		// Ignore all other commands (for now)
	}
}

Mesh* loadObj(const char* filename) {
	FileReader fileReader(filename, FileReader::Asset);
	void* data = fileReader.readAll();
	int length = fileReader.size() + 1;
	char* source = Memory::scratchPad<char>(length);
	memcpy(source, data, length);
	source[length] = 0;
	
	Mesh* mesh = Memory::allocate<Mesh>();
	
	int vertices = countVertices(source);
	mesh->vertices = Memory::allocate<float>(vertices * 5);
	mesh->curVertex = mesh->vertices;
	int faces = countFaces(source);
	mesh->indices = Memory::allocate<int>(faces * 3);
	mesh->curIndex = mesh->indices;
	mesh->numUVs = countUVs(source);
	mesh->uvs = Memory::allocate<float>(mesh->numUVs * 2);
	mesh->curUV = mesh->uvs;
	
	mesh->numVertices = 0;
	mesh->numFaces = 0;
	
	int index = 0;
	char line[maxTokenLength];
	tokenize(source, '\n', index, line);
	
	while (line[0] != 0) {
		parseLine(mesh, line);
		tokenize(source, '\n', index, line);
	}
	
	return mesh;
}
