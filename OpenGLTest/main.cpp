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
#include "utils.cpp"
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

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	bool res = load_obj("C:\\Users\\serpi\\Documents\\CppProjects\\OpenGLTest\\OpenGLTest\\Intergalactic_Spaceship.obj", vertices, uvs, normals);

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uv_buffer;
	glGenBuffers(1, &uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);

	// ������� ����� ������������ ������� ������, ��� �������� ����
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// ������� � ��������������� ���� ��������� ���������
	GLuint program_id = load_shaders("vertex_standart.glsl", "fragment_standart.glsl");

	// �������� ����� ���������� � �������
	GLuint matrix_id = glGetUniformLocation(program_id, "mvp");

	// �������� ���� �������
	glEnable(GL_DEPTH_TEST);
	// �������� ����� ���������� ������ � ���, ������, ���� �� ��������� ����� � ������, ��� ����������
	glDepthFunc(GL_LESS);

	GLuint texture_id = load_bmp("space_ship.bmp");

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	glfwSetCursorPos(window, float(w) / 2.0f, float(h) / 2.0f);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	
	float delta_time = 0;
	float last_time = 0;
	vec3 position = vec3(-5, 0, 0);
	float x_angle = 0;
	float y_angle = 0;
	const float mouse_speed = 0.04f;
	const float fly_speed = 1.0f;
	
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0) {

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		delta_time = float(glfwGetTime()) - last_time;
		last_time = float(glfwGetTime());

		double dx, dy;
		glfwGetWindowSize(window, &w, &h);
		glfwGetCursorPos(window, &dx, &dy);
		glfwSetCursorPos(window, float(w)/2.0f, float(h)/2.0f);
		dx -= float(w) / 2.0f;
		dy -= float(h) / 2.0f;
		x_angle += float(-dx) * delta_time * mouse_speed;
		y_angle += float(dy) * delta_time * mouse_speed;

		vec3 right = vec4(cosf(x_angle - pi<float>()/2.0f), 0, sinf(x_angle - pi<float>() / 2.0f), 1);
		vec3 forward = glm::rotate(mat4(1.0f), y_angle, vec3(0, 0, 1)) * vec4(1, 0, 0, 0) * glm::rotate(mat4(1.0f), x_angle, vec3(0, 1, 0));
		vec3 up = glm::cross(right, forward);
		
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			position += fly_speed * forward * delta_time;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			position += fly_speed * -forward * delta_time;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			position += fly_speed * -right * delta_time;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			position += fly_speed * right * delta_time;

		// ������������ ������� : 45&deg; ���� ������, 4:3 ����������� ������, �������� : 0.1 ���� <-> 100 ������
		mat4 projection = glm::perspective(radians(75.0f), 4.0f / 3.0f, 0.1f, 100.0f);

		// ���, ��� ����������
		mat4 view = glm::lookAt(
			position, // ������ ��������� � ������� �����������
			position + forward, // � ���������� � ������ ���������
			up  // "������" ��������� ������
		);

		// ������� ������ : ��������� ������� (������ ��������� � ������ ���������)
		mat4 model = mat4(1.0f);  // ������������� ��� ������ ������

		// �������� ������� ModelViewProjection, ������� �������� ����������� ������������ ����� ���� ������
		mat4 mvp = projection * view * model; // �������, ��� ��������� ������� ������������� � �������� �������

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
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glVertexAttribPointer(
			1,                                // �������. ����� ������������� ��������� 1, �� �������, ����� ��� �������� ��������� � layout � �������..
			2,                                // ������
			GL_FLOAT,                         // ���
			GL_FALSE,                         // ������������?
			0,                                // ���
			(void*)0                          // ��������
		);

		// ������������� ��� ������ �������
		glUseProgram(program_id);
		
		// ������� �����������
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 �������� ������������ � 0. -> 12 ������������� -> 6 ������.

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// ���������� ������
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}