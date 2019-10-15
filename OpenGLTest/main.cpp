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

	// Массив 3 векторов, которые являются вершинами треугольника
	static const GLfloat g_vertex_buffer_data[] = {
	   -1.0f, -1.0f, 0.0f,
	   1.0f, -1.0f, 0.0f,
	   0.0f,  1.0f, 0.0f,
	};

	// Это будет идентификатором нашего буфера вершин
	GLuint vertex_buffer;

	// Создадим 1 буфер и поместим в переменную vertexbuffer его идентификатор
	glGenBuffers(1, &vertex_buffer);

	// Сделаем только что созданный буфер текущим
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// Передадим информацию о вершинах в OpenGL
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// Включим режим отслеживания нажатия клавиш, для проверки ниже
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Создать и откомпилировать нашу шейдерную программу
	GLuint program_id = load_shaders("vertex_standart.glsl", "fragment_standart.glsl");

	// Проекционная матрица : 45&deg; поле обзора, 4:3 соотношение сторон, диапазон : 0.1 юнит <-> 100 юнитов
	mat4 projection = glm::perspective(radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	// Или, для ортокамеры
	mat4 view = glm::lookAt(
		vec3(4, 3, 3), // Камера находится в мировых координатах (4,3,3)
		vec3(0, 0, 0), // И направлена в начало координат
		vec3(0, 1, 0)  // "Голова" находится сверху
	);

	// Матрица модели : единичная матрица (Модель находится в начале координат)
	mat4 model = mat4();  // Индивидуально для каждой модели

	// Итоговая матрица ModelViewProjection, которая является результатом перемножения наших трех матриц
	mat4 mvp = projection * view * model; // Помните, что умножение матрицы производиться в обратном порядке

	// Получить хэндл переменной в шейдере
	GLuint matrix_id = glGetUniformLocation(program_id, "mvp");
	
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0) {
		
		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

		// Устанавливаем наш шейдер текущим
		glUseProgram(program_id);
		
		// Вывести треугольник!
		glDrawArrays(GL_TRIANGLES, 0, 3); // Начиная с вершины 0, всего 3 вершины -> один треугольник

		glDisableVertexAttribArray(0);

		// Сбрасываем буферы
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}