#include <cstdio>
#include <cstdlib>
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glew32s.lib")
#include <GL/glew.h>
#include <GL/GL.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/freeglut.h>
#include <windows.h>
#include "shaders.cpp"
#include <thread>
#include <chrono>
#include <clocale>
using namespace glm;
using namespace std::chrono_literals;

int main() {
	setlocale(LC_CTYPE, "rus"); // ����� ������� ��������� ������
	// �������������� GLFW
	if (!glfwInit()) {
		fprintf(stderr, "������ ��� ������������� GLFW\n");
		return -1;
	}
	
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x �����������
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // �� ����� ������������ OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // �� �� ����� ������ OpenGL

	// ������� ���� � ������� � ��� �������� OpenGL
	GLFWwindow* window; // (� ���������������� �������� ���� ��� ���������� �������� ����������)
	window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "���������� ������� ���� GLFW. ���� � ��� Intel GPU, �� �� �� ������������ ������ 3.3. ���������� ������ ������ ��� OpenGL 2.1.n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	
	// �������������� GLEW
	glewExperimental = true; // ���� ��������� � Core-������ OpenGL
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "���������� ���������������� GLEWn");
		return -1;
	}

	// ������� VAO
	GLuint vertex_array_id;
	glGenVertexArrays(1, &vertex_array_id);
	glBindVertexArray(vertex_array_id);

	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f,-1.0f,-1.0f, // ����������� 1 : ������
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f, // ����������� 1 : �����
		1.0f, 1.0f,-1.0f, // ����������� 2 : ������
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f, // ����������� 2 : �����
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
	};

	// ���� ���� ��� ������ �������
	static const GLfloat g_color_buffer_data[] = {
		0.583f,  0.771f,  0.014f,
		0.609f,  0.115f,  0.436f,
		0.327f,  0.483f,  0.844f,
		0.822f,  0.569f,  0.201f,
		0.435f,  0.602f,  0.223f,
		0.310f,  0.747f,  0.185f,
		0.597f,  0.770f,  0.761f,
		0.559f,  0.436f,  0.730f,
		0.359f,  0.583f,  0.152f,
		0.483f,  0.596f,  0.789f,
		0.559f,  0.861f,  0.639f,
		0.195f,  0.548f,  0.859f,
		0.014f,  0.184f,  0.576f,
		0.771f,  0.328f,  0.970f,
		0.406f,  0.615f,  0.116f,
		0.676f,  0.977f,  0.133f,
		0.971f,  0.572f,  0.833f,
		0.140f,  0.616f,  0.489f,
		0.997f,  0.513f,  0.064f,
		0.945f,  0.719f,  0.592f,
		0.543f,  0.021f,  0.978f,
		0.279f,  0.317f,  0.505f,
		0.167f,  0.620f,  0.077f,
		0.347f,  0.857f,  0.137f,
		0.055f,  0.953f,  0.042f,
		0.714f,  0.505f,  0.345f,
		0.783f,  0.290f,  0.734f,
		0.722f,  0.645f,  0.174f,
		0.302f,  0.455f,  0.848f,
		0.225f,  0.587f,  0.040f,
		0.517f,  0.713f,  0.338f,
		0.053f,  0.959f,  0.120f,
		0.393f,  0.621f,  0.362f,
		0.673f,  0.211f,  0.457f,
		0.820f,  0.883f,  0.371f,
		0.982f,  0.099f,  0.879f
	};

	// ��� ����� ��������������� ������ ������ ������
	GLuint vertex_buffer;

	// �������� 1 ����� � �������� � ���������� vertexbuffer ��� �������������
	glGenBuffers(1, &vertex_buffer);

	// ������� ������ ��� ��������� ����� �������
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// ��������� ���������� � �������� � OpenGL
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	// ������� ����� ������������ ������� ������, ��� �������� ����
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// ������� � ��������������� ���� ��������� ���������
	GLuint program_id = load_shaders("vertex_standart.glsl", "fragment_standart.glsl");

	// ������������ ������� : 45&deg; ���� ������, 4:3 ����������� ������, �������� : 0.1 ���� <-> 100 ������
	mat4 projection = glm::perspective(radians(90.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	// ���, ��� ����������
	mat4 view = glm::lookAt(
		vec3(4, 3, -3), // ������ ��������� � ������� ����������� (4,3,3)
		vec3(0, 0, 0), // � ���������� � ������ ���������
		vec3(0, 1, 0)  // "������" ��������� ������
	);

	// ������� ������ : ��������� ������� (������ ��������� � ������ ���������)
	mat4 model = mat4(1.0f);  // ������������� ��� ������ ������

	// �������� ������� ModelViewProjection, ������� �������� ����������� ������������ ����� ���� ������
	mat4 mvp = projection * view * model; // �������, ��� ��������� ������� ������������� � �������� �������

	// �������� ����� ���������� � �������
	GLuint matrix_id = glGetUniformLocation(program_id, "mvp");

	// �������� ���� �������
	glEnable(GL_DEPTH_TEST);
	// �������� ����� ���������� ������ � ���, ������, ���� �� ��������� ����� � ������, ��� ����������
	glDepthFunc(GL_LESS);
	
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0) {

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// �������� ���� ������������� � ������� ������
		// ��� �������� � �������� �����, ��������� ������ ������ ����� ����� ������ MVP-������� (��� ������� ����� M)
		glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);
		
		// ���������, ��� ������ ������� ��������� ����� �������
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(
			0,                  // ������� 0. ��������� �� ���� ����� ���������� � �����, ����������� ��������.
			3,                  // ������
			GL_FLOAT,           // ���
			GL_FALSE,           // ���������, ��� �������� �� �������������
			0,                  // ���
			(void*)0            // �������� ������� � ������
		);
		

		// ������ ����� ��������� - �����
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,                                // �������. ����� ������������� ��������� 1, �� �������, ����� ��� �������� ��������� � layout � �������..
			3,                                // ������
			GL_FLOAT,                         // ���
			GL_FALSE,                         // ������������?
			0,                                // ���
			(void*)0                          // ��������
		);

		// ������������� ��� ������ �������
		glUseProgram(program_id);
		
		// ������� �����������
		glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 �������� ������������ � 0. -> 12 ������������� -> 6 ������.

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// ���������� ������
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}