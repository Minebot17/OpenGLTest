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
	setlocale(LC_CTYPE, "rus"); // вызов функции настройки локали
	// Инициализируем GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Ошибка при инициализации GLFW\n");
		return -1;
	}
	
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x Сглаживание
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Мы хотим использовать OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Мы не хотим старый OpenGL

	// Открыть окно и создать в нем контекст OpenGL
	GLFWwindow* window; // (В сопроводительном исходном коде эта переменная является глобальной)
	window = glfwCreateWindow(800, 600, "Test", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Невозможно открыть окно GLFW. Если у вас Intel GPU, то он не поддерживает версию 3.3. Попробуйте версию уроков для OpenGL 2.1.n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	
	// Инициализируем GLEW
	glewExperimental = true; // Флаг необходим в Core-режиме OpenGL
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Невозможно инициализировать GLEWn");
		return -1;
	}

	// Создаем VAO
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

	// Включим режим отслеживания нажатия клавиш, для проверки ниже
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Создать и откомпилировать нашу шейдерную программу
	GLuint program_id = load_shaders("vertex_standart.glsl", "fragment_standart.glsl");

	// Получить хэндл переменной в шейдере
	GLuint matrix_id = glGetUniformLocation(program_id, "mvp");

	// Включить тест глубины
	glEnable(GL_DEPTH_TEST);
	// Фрагмент будет выводиться только в том, случае, если он находится ближе к камере, чем предыдущий
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

		// Проекционная матрица : 45&deg; поле обзора, 4:3 соотношение сторон, диапазон : 0.1 юнит <-> 100 юнитов
		mat4 projection = glm::perspective(radians(75.0f), 4.0f / 3.0f, 0.1f, 100.0f);

		// Или, для ортокамеры
		mat4 view = glm::lookAt(
			position, // Камера находится в мировых координатах
			position + forward, // И направлена в начало координат
			up  // "Голова" находится сверху
		);

		// Матрица модели : единичная матрица (Модель находится в начале координат)
		mat4 model = mat4(1.0f);  // Индивидуально для каждой модели

		// Итоговая матрица ModelViewProjection, которая является результатом перемножения наших трех матриц
		mat4 mvp = projection * view * model; // Помните, что умножение матрицы производиться в обратном порядке

		// Передать наши трансформации в текущий шейдер
		// Это делается в основном цикле, поскольку каждая модель будет иметь другую MVP-матрицу (как минимум часть M)
		glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);
		
		// Указываем, что первым буфером атрибутов будут вершины
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(
			0,                  // Атрибут 0. Подробнее об этом будет рассказано в части, посвященной шейдерам.
			3,                  // Размер
			GL_FLOAT,           // Тип
			GL_FALSE,           // Указывает, что значения не нормализованы
			0,                  // Шаг
			(void*)0            // Смещение массива в буфере
		);
		

		// Второй буфер атрибутов - цвета
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glVertexAttribPointer(
			1,                                // Атрибут. Здесь необязательно указывать 1, но главное, чтобы это значение совпадало с layout в шейдере..
			2,                                // Размер
			GL_FLOAT,                         // Тип
			GL_FALSE,                         // Нормализован?
			0,                                // Шаг
			(void*)0                          // Смещение
		);

		// Устанавливаем наш шейдер текущим
		glUseProgram(program_id);
		
		// Вывести треугольник
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 индексов начинающихся с 0. -> 12 треугольников -> 6 граней.

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Сбрасываем буферы
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}