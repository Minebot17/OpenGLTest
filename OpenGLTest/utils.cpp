#include <cstdio>
#include <string>
#include <filesystem>
#include <fstream>
#include <istream>
#include <sstream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
using std::vector;
using namespace glm;

static GLuint load_shaders(const char* vertex_file_path, const char* fragment_file_path) {

	// ������� �������
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// ��������� ��� ���������� ������� �� �����
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}

	// ��������� ��� ������������ ������� �� �����
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// ����������� ��������� ������
	printf("���������� �������: %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// ��������� �������� ���������� �������
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		fprintf(stdout, "%sn", &VertexShaderErrorMessage[0]);
	}

	// ����������� ����������� ������
	printf("���������� �������: %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// ��������� ����������� ������
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
	}

	// ������� ��������� ��������� � ����������� ������� � ���
	fprintf(stdout, "������� ��������� ��������� � ����������� ������� � ���\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// ��������� ��������� ���������
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static GLuint load_bmp(const char* path, bool sRGB) {
	// ������, ����������� �� ��������� BMP-�����
	unsigned char header[54]; // ������ BMP-���� ���������� � ���������, ������ � 54 �����
	unsigned int dataPos;     // �������� ������ � ����� (������� ������)
	unsigned int width, height;
	unsigned int imageSize;   // ������ ����������� = ������ * ������ * 3
	// RGB-������, ���������� �� �����
	unsigned char* data;
	
	FILE* file;
	fopen_s(&file, path, "rb");
	if (!file) {
		printf("����������� �� ����� ���� �������\n");
		return 0;
	}

	if (fread(header, 1, 54, file) != 54) { // ���� �� ��������� ������ 54 ����, ������ �������� ��������
		printf("������������ BMP-����n");
		return false;
	}

	if (header[0] != 'B' || header[1] != 'M') {
		printf("������������ BMP-����n");
		return 0;
	}

	// ������ ����������� ������
	dataPos = *(int*) & (header[0x0A]); // �������� ������ ����������� � �����
	imageSize = *(int*) & (header[0x22]); // ������ ����������� � ������
	width = *(int*) & (header[0x12]); // ������
	height = *(int*) & (header[0x16]); // ������

	// ��������� BMP-����� ����� ������� ���� imageSize � dataPos, ������� �������� ��
	if (imageSize == 0)    imageSize = width * height * 3; // ������ * ������ * 3, ��� 3 - 3 ���������� ����� (RGB)
	if (dataPos == 0)      dataPos = 54; // � ����� ������, ������ ����� ��������� ����� �� ����������

	// ������� �����
	data = new unsigned char[imageSize];

	// ��������� ������ �� ����� � �����
	fread(data, 1, imageSize, file);

	// ��������� ����, ��� ��� ������ �� ��� �� �����
	fclose(file);

	// �������� ���� �������� OpenGL
	GLuint textureID;
	glGenTextures(1, &textureID);

	// ������� ��������� �������� �������, ����� ������� ��� ��������� ������� ����� �������� ������ � ���� ���������
	glBindTexture(GL_TEXTURE_2D, textureID);

	// ��������� ����������� OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, sRGB ? GL_SRGB : GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// ����� ����������� �������������, �� �� ���������� ������� �������� ����������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// ����� ����������� �����������, �� �� ���������� �������� ���������� 2� ��������, � ������� ����� ����������� �������� ����������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// � ���������� ������
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

static bool load_obj(const char* path, vector<vec3>& vertices, vector<vec2>& uvs, vector<vec3>& normals) {
	vector<vec3> ns_vertices;
	vector<vec2> ns_uvs;
	vector<vec3> ns_normals;

	FILE* file = fopen(path, "r");
	if (file == nullptr) {
		printf("Impossible to open the file !\n");
		return false;
	}
	
	while (true) {
		char buffer[256];
		const int result = fscanf(file, "%s", buffer);
		if (result == EOF)
			break;

		if (strcmp(buffer, "v") == 0) {
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			ns_vertices.push_back(vertex);
		}
		else if (strcmp(buffer, "vt") == 0) {
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			ns_uvs.push_back(uv);
		}
		else if (strcmp(buffer, "vn") == 0) {
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			ns_normals.push_back(normal);
		}
		else if (strcmp(buffer, "f") == 0) {
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			vertices.push_back(ns_vertices[vertexIndex[0] - 1]);
			vertices.push_back(ns_vertices[vertexIndex[1] - 1]);
			vertices.push_back(ns_vertices[vertexIndex[2] - 1]);
			uvs.push_back(ns_uvs[uvIndex[0] - 1]);
			uvs.push_back(ns_uvs[uvIndex[1] - 1]);
			uvs.push_back(ns_uvs[uvIndex[2] - 1]);
			normals.push_back(ns_normals[normalIndex[0] - 1]);
			normals.push_back(ns_normals[normalIndex[1] - 1]);
			normals.push_back(ns_normals[normalIndex[2] - 1]);
		}
	}
	return true;
}

static void compute_tangent_basis (
	// inputs
    std::vector<glm::vec3> &vertices,
    std::vector<glm::vec2> &uvs,
    std::vector<glm::vec3> &normals,
    // outputs			   
    std::vector<glm::vec3> &tangents,
    std::vector<glm::vec3> &bitangents
) {
	for (int i = 0; i < vertices.size(); i += 3) {

		// Shortcuts for vertices
		glm::vec3& v0 = vertices[i + 0];
		glm::vec3& v1 = vertices[i + 1];
		glm::vec3& v2 = vertices[i + 2];

		// Shortcuts for UVs
		glm::vec2& uv0 = uvs[i + 0];
		glm::vec2& uv1 = uvs[i + 1];
		glm::vec2& uv2 = uvs[i + 2];

		// Edges of the triangle : postion delta
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

		// Set the same tangent for all three vertices of the triangle.
		// They will be merged later, in vboindexer.cpp
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		// Same thing for binormals
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
	}
}

static unsigned int load_cubemap(vector<std::string> faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		// ������, ����������� �� ��������� BMP-�����
		unsigned char header[54]; // ������ BMP-���� ���������� � ���������, ������ � 54 �����
		unsigned int dataPos;     // �������� ������ � ����� (������� ������)
		unsigned int width, height;
		unsigned int imageSize;   // ������ ����������� = ������ * ������ * 3
		// RGB-������, ���������� �� �����
		unsigned char* data;

		FILE* file;
		fopen_s(&file, faces[i].c_str(), "rb");
		if (!file) {
			printf("����������� �� ����� ���� �������\n");
			return 0;
		}

		if (fread(header, 1, 54, file) != 54) { // ���� �� ��������� ������ 54 ����, ������ �������� ��������
			printf("������������ BMP-����n");
			return false;
		}

		if (header[0] != 'B' || header[1] != 'M') {
			printf("������������ BMP-����n");
			return 0;
		}

		// ������ ����������� ������
		dataPos = *(int*) & (header[0x0A]); // �������� ������ ����������� � �����
		imageSize = *(int*) & (header[0x22]); // ������ ����������� � ������
		width = *(int*) & (header[0x12]); // ������
		height = *(int*) & (header[0x16]); // ������

		// ��������� BMP-����� ����� ������� ���� imageSize � dataPos, ������� �������� ��
		if (imageSize == 0)    imageSize = width * height * 3; // ������ * ������ * 3, ��� 3 - 3 ���������� ����� (RGB)
		if (dataPos == 0)      dataPos = 54; // � ����� ������, ������ ����� ��������� ����� �� ����������

		// ������� �����
		data = new unsigned char[imageSize];

		// ��������� ������ �� ����� � �����
		fread(data, 1, imageSize, file);

		// ��������� ����, ��� ��� ������ �� ��� �� �����
		fclose(file);
		
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_SRGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data
			);
		}
		else
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}