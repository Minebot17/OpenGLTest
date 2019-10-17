#include <cstdio>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/GL.h>

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

static GLuint load_bmp(const char* path) {
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// ����� ����������� �������������, �� �� ���������� ������� �������� ����������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// ����� ����������� �����������, �� �� ���������� �������� ���������� 2� ��������, � ������� ����� ����������� �������� ����������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// � ���������� ������
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}