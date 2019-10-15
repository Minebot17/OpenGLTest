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
using namespace glm;
using namespace std::chrono_literals;

int main() {
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

	// ������ 3 ��������, ������� �������� ��������� ������������
	static const GLfloat g_vertex_buffer_data[] = {
	   -1.0f, -1.0f, 0.0f,
	   1.0f, -1.0f, 0.0f,
	   0.0f,  1.0f, 0.0f,
	};

	// ��� ����� ��������������� ������ ������ ������
	GLuint vertex_buffer;

	// �������� 1 ����� � �������� � ���������� vertexbuffer ��� �������������
	glGenBuffers(1, &vertex_buffer);

	// ������� ������ ��� ��������� ����� �������
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// ��������� ���������� � �������� � OpenGL
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// ������� ����� ������������ ������� ������, ��� �������� ����
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// ������� � ��������������� ���� ��������� ���������
	GLuint program_id = load_shaders("vertex_standart.glsl", "fragment_standart.glsl");

	// ������������ ������� : 45&deg; ���� ������, 4:3 ����������� ������, �������� : 0.1 ���� <-> 100 ������
	mat4 projection = glm::perspective(radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	// ���, ��� ����������
	mat4 view = glm::lookAt(
		vec3(4, 3, 3), // ������ ��������� � ������� ����������� (4,3,3)
		vec3(0, 0, 0), // � ���������� � ������ ���������
		vec3(0, 1, 0)  // "������" ��������� ������
	);

	// ������� ������ : ��������� ������� (������ ��������� � ������ ���������)
	mat4 model = mat4();  // ������������� ��� ������ ������

	// �������� ������� ModelViewProjection, ������� �������� ����������� ������������ ����� ���� ������
	mat4 mvp = projection * view * model; // �������, ��� ��������� ������� ������������� � �������� �������

	// �������� ����� ���������� � �������
	GLuint matrix_id = glGetUniformLocation(program_id, "mvp");
	
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

		// ������������� ��� ������ �������
		glUseProgram(program_id);
		
		// ������� �����������!
		glDrawArrays(GL_TRIANGLES, 0, 3); // ������� � ������� 0, ����� 3 ������� -> ���� �����������

		glDisableVertexAttribArray(0);

		// ���������� ������
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}