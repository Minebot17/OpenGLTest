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
	window = glfwCreateWindow(1600, 1200, "Test", nullptr, nullptr);
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

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	//glfwSetCursorPos(window, float(w) / 2.0f, float(h) / 2.0f);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	GLuint shadow_framebuffer_id = 0;
	glGenFramebuffers(1, &shadow_framebuffer_id);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer_id);

	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	GLuint shadow_texture;
	glGenTextures(1, &shadow_texture);
	glBindTexture(GL_TEXTURE_2D, shadow_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_texture, 0);
	glDrawBuffer(GL_NONE); // No color buffer is drawn to.

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("Framebuffer isn't ok");

	GLuint shadow_map_program_id = load_shaders("shadow_map_vertex.glsl", "shadow_map_fragment.glsl");
	GLuint shadow_mvp_location = glGetUniformLocation(shadow_map_program_id, "shadow_mvp");

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	GLuint framebuffer_id = 0;
	glGenFramebuffers(1, &framebuffer_id);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

	// The texture we're going to render to
	GLuint rendered_texture;
	glGenTextures(1, &rendered_texture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, rendered_texture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// The depth buffer
	GLuint depth_renderbuffer;
	glGenRenderbuffers(1, &depth_renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendered_texture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("Framebuffer isn't ok");

	// The fullscreen quad's FBO
	GLuint quad_vertex_array_id;
	glGenVertexArrays(1, &quad_vertex_array_id);
	glBindVertexArray(quad_vertex_array_id);

	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};

	static const GLfloat g_quad_uv_buffer_data[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};
	
	GLuint quad_vertex_buffer;
	glGenBuffers(1, &quad_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint quad_uv_buffer;
	glGenBuffers(1, &quad_uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_uv_buffer_data), g_quad_uv_buffer_data, GL_STATIC_DRAW);

	// Create and compile our GLSL program from the shaders
	GLuint post_program_id = load_shaders("post_vertex.glsl", "post_fragment.glsl");
	GLuint rendered_texture_location = glGetUniformLocation(post_program_id, "rendered_texture");
	GLuint post_time_location = glGetUniformLocation(post_program_id, "time");
	GLuint post_shadow_texture_location = glGetUniformLocation(post_program_id, "shadow_texture");
	GLuint post_gamma_location = glGetUniformLocation(post_program_id, "gamma");

	vector<vec3> sky_one_vertices = {
		vec3(-1,-1,-1),
		vec3(1,-1,-1),
		vec3(1,1,-1),
		vec3(-1,1,-1),
		vec3(-1,-1,1),
		vec3(1,-1,1),
		vec3(1,1,1),
		vec3(-1,1,1),
	};
	vector<int> sky_indexes = { 2,3,0,0,1,2,2,1,6,1,5,6,4,3,7,4,0,3,4,7,6,6,5,4,3,2,6,7,3,6,5,1,0,4,5,0 };
	vector<vec3> sky_vertices;
	for (int i = 0; i < sky_indexes.size(); i++)
		sky_vertices.push_back(sky_one_vertices[sky_indexes[i]]);
	
	GLuint sky_vao;
	glGenVertexArrays(1, &sky_vao);
	glBindVertexArray(sky_vao);

	GLuint sky_shader_id = load_shaders("sky_vertex.glsl", "sky_fragment.glsl");
	GLuint sky_mvp_location = glGetUniformLocation(sky_shader_id, "mvp");
	GLuint sky_cubemap_location = glGetUniformLocation(sky_shader_id, "skybox");

	GLuint sky_vertex_buffer;
	glGenBuffers(1, &sky_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sky_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sky_vertices.size() * sizeof(glm::vec3), &sky_vertices[0], GL_STATIC_DRAW);
	
	// Создаем VAO
	GLuint vertex_array_id;
	glGenVertexArrays(1, &vertex_array_id);
	glBindVertexArray(vertex_array_id);

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	std::vector<vec3> tangents;
	std::vector<vec3> bitangents;
	load_obj("C:\\Users\\serpi\\Documents\\CppProjects\\OpenGLTest\\OpenGLTest\\Intergalactic_Spaceship.obj", vertices, uvs, normals);
	compute_tangent_basis(vertices, uvs, normals, tangents, bitangents);
	
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uv_buffer;
	glGenBuffers(1, &uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);

	GLuint normal_buffer;
	glGenBuffers(1, &normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);

	GLuint tangent_buffer;
	glGenBuffers(1, &tangent_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, tangent_buffer);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);

	GLuint bitangent_buffer;
	glGenBuffers(1, &bitangent_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, bitangent_buffer);
	glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0], GL_STATIC_DRAW);

	// Включим режим отслеживания нажатия клавиш, для проверки ниже
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Создать и откомпилировать нашу шейдерную программу
	GLuint program_id = load_shaders("vertex_standart.glsl", "fragment_standart.glsl");

	// Получить хэндл переменной в шейдере
	GLuint matrix_id = glGetUniformLocation(program_id, "mvp");
	GLuint model_matrix_id = glGetUniformLocation(program_id, "model");
	GLuint view_matrix_id = glGetUniformLocation(program_id, "view");
	GLuint mv3x3_id = glGetUniformLocation(program_id, "mv3x3");
	GLuint light_color_id = glGetUniformLocation(program_id, "lightColor"); 
	GLuint light_power_id = glGetUniformLocation(program_id, "lightPower");
	GLuint light_position_worldspace_id = glGetUniformLocation(program_id, "lightDirection_worldspace");
	GLuint depth_bias_mvp_id = glGetUniformLocation(program_id, "depthBiasMVP");
	GLuint time_id = glGetUniformLocation(program_id, "time");

	GLuint texture_location = glGetUniformLocation(program_id, "textureSampler");
	GLuint normal_location = glGetUniformLocation(program_id, "normalSampler");
	GLuint specular_location = glGetUniformLocation(program_id, "specularSampler");
	GLuint shadow_location = glGetUniformLocation(program_id, "shadowSampler");

	// Фрагмент будет выводиться только в том, случае, если он находится ближе к камере, чем предыдущий
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);

	GLuint texture_id = load_bmp("space_ship.bmp", true);
	GLuint normal_id = load_bmp("space_ship_normals.bmp", false);
	GLuint specular_id = load_bmp("space_ship_specular.bmp", false);

	vector<std::string> faces {
		"sky_neg_x.bmp",
		"sky_pos_x.bmp",
		"sky_pos_y.bmp",
		"sky_neg_y.bmp",
		"sky_pos_z.bmp",
		"sky_neg_z.bmp"
	};
	unsigned int cubemap_texture = load_cubemap(faces);
	
	float delta_time = 0;
	float last_time = 0;
	vec3 position = vec3(-5, 0, 0);
	float x_angle = 0;
	float y_angle = 0;
	const float mouse_speed = 0.04f;
	const float fly_speed = 4.0f;
	
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0) {

		delta_time = float(glfwGetTime()) - last_time;
		last_time = float(glfwGetTime());

		double dx, dy;
		glfwGetWindowSize(window, &w, &h);
		glfwGetCursorPos(window, &dx, &dy);
		//glfwSetCursorPos(window, float(w)/2.0f, float(h)/2.0f);
		dx -= float(w) / 2.0f;
		dy -= float(h) / 2.0f;
		x_angle += float(dx) * delta_time * mouse_speed;
		y_angle += float(-dy) * delta_time * mouse_speed;

		vec3 left = vec4(cosf(x_angle - pi<float>()/2.0f), 0, sinf(x_angle - pi<float>() / 2.0f), 1);
		vec3 forward = glm::rotate(mat4(1.0f), y_angle, vec3(0, 0, 1)) * vec4(1, 0, 0, 0) * glm::rotate(mat4(1.0f), x_angle, vec3(0, 1, 0));
		vec3 up = glm::cross(forward, left);
		
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			position += fly_speed * forward * delta_time;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			position += fly_speed * -forward * delta_time;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			position += fly_speed * left * delta_time;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			position += fly_speed * -left * delta_time;
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
			position += fly_speed * -up * delta_time;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			position += fly_speed * up * delta_time;
		
		// Проекционная матрица : 45&deg; поле обзора, 4:3 соотношение сторон, диапазон : 0.1 юнит <-> 100 юнитов
		mat4 projection = glm::perspective(radians(75.0f), 4.0f / 3.0f, 0.1f, 100.0f);

		// Или, для ортокамеры
		mat4 view = glm::lookAt(
			vec3(rotate(mat4(1.0f), float(glfwGetTime() / 4.0f), vec3(0.25f, 1, 0)) * vec4(0, 4, 2, 0)), // Камера находится в мировых координатах
			vec3(rotate(mat4(1.0f), float(glfwGetTime() / 4.0f), vec3(0.25f, 1, 0)) * vec4(0, 0, -2, 0)),
			vec3(rotate(mat4(1.0f), float(glfwGetTime() / 4.0f), vec3(0.25f, 1, 0)) * vec4(0, 0.5f, -0.5f, 0))  // "Голова" находится сверху
		);

		// Матрица модели : единичная матрица (Модель находится в начале координат)
		mat4 model = glm::rotate(mat4(1.0f), float(glfwGetTime() / 4.0f), vec3(0.25f, 1, 0)) * glm::translate(mat4(1.0f), vec3(0, 0, -2));  // Индивидуально для каждой модели

		// Итоговая матрица ModelViewProjection, которая является результатом перемножения наших трех матриц
		mat4 mvp = projection * view * model; // Помните, что умножение матрицы производиться в обратном порядке

		// Передать наши трансформации в текущий шейдер
		// Это делается в основном цикле, поскольку каждая модель будет иметь другую MVP-матрицу (как минимум часть M)
		vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
		vec3 light_position = vec3(0.454f, 0.536f, 0.712f);
		vec3 light_direction_worldspace = normalize(light_position);
		//vec3 right_light = vec4(sinf(glfwGetTime() - pi<float>() / 2.0f), 0, cosf(glfwGetTime() - pi<float>() / 2.0f), 1);
		//vec3 up_light = glm::cross(right_light, light_direction_worldspace);
		float light_power = 2.0f;
		mat3 mv3x3 = mat3(model * view);

		glm::mat4 shadow_projection_matrix = glm::ortho<float>(-10, 10, -10, 10, -5,8);
		glm::mat4 shadow_view_matrix = glm::lookAt(light_position, glm::vec3(0, 0, 0), vec3(0, 1, 0));
		glm::mat4 shadow_model_matrix = model;
		glm::mat4 shadow_mvp = shadow_projection_matrix * shadow_view_matrix * shadow_model_matrix;
		const glm::mat4 bias_matrix(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
		);
		glm::mat4 depth_bias_mvp = bias_matrix * shadow_mvp;

		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer_id);
		glViewport(0, 0, 4096, 4096); // Render on the whole framebuffer, complete from the lower left corner to the upper righ
		glBindVertexArray(vertex_array_id);

		// Устанавливаем наш шейдер текущим
		glUseProgram(shadow_map_program_id);
		
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(shadow_mvp_location, 1, GL_FALSE, &shadow_mvp[0][0]);

		glClearColor(0.0f, 0.0f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

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
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 индексов начинающихся с 0. -> 12 треугольников -> 6 граней.
		glDisableVertexAttribArray(0);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
		glViewport(0, 0, w, h); // Render on the whole framebuffer, complete from the lower left corner to the upper righ
		glBindVertexArray(vertex_array_id);

		// Устанавливаем наш шейдер текущим
		glUseProgram(program_id);
		
		glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(depth_bias_mvp_id, 1, GL_FALSE, &depth_bias_mvp[0][0]);
		glUniformMatrix3fv(mv3x3_id, 1, GL_FALSE, &mv3x3[0][0]);
		glUniform3f(light_position_worldspace_id, light_direction_worldspace.x, light_direction_worldspace.y, light_direction_worldspace.z);
		glUniform3f(light_color_id, light_color.x, light_color.y, light_color.z);
		glUniform1f(light_power_id, light_power);
		glUniform1f(time_id, glfwGetTime());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glUniform1i(texture_location, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normal_id);
		glUniform1i(normal_location, 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, specular_id);
		glUniform1i(specular_location, 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadow_texture);
		glUniform1i(shadow_location, 3);

		glClearColor(0.0f, 0.0f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		
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

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glVertexAttribPointer(
			2,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*) 0
		);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, tangent_buffer);
		glVertexAttribPointer(
			3,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, bitangent_buffer);
		glVertexAttribPointer(
			4,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		
		// Вывести треугольник
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 индексов начинающихся с 0. -> 12 треугольников -> 6 граней.

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		mat4 sky_view = mat4(mat3(view));
		mat4 sky_mvp = projection * sky_view;
		glDepthMask(GL_FALSE);
		
		glBindVertexArray(sky_vao);
		glUseProgram(sky_shader_id);
		glUniformMatrix4fv(sky_mvp_location, 1, GL_FALSE, &sky_mvp[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubemap_texture);
		glUniform1i(sky_cubemap_location, 0);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, sky_vertex_buffer);
		glVertexAttribPointer(
			0,                  // Атрибут 0. Подробнее об этом будет рассказано в части, посвященной шейдерам.
			3,                  // Размер
			GL_FLOAT,           // Тип
			GL_FALSE,           // Указывает, что значения не нормализованы
			0,                  // Шаг
			(void*)0            // Смещение массива в буфере
		);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDisableVertexAttribArray(0);
		glDepthMask(GL_TRUE);

		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		
		glBindVertexArray(quad_vertex_array_id);
		glUseProgram(post_program_id);
		glDisable(GL_DEPTH_TEST);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rendered_texture);
		glUniform1i(rendered_texture_location, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadow_texture);
		glUniform1i(post_shadow_texture_location, 1);
		
		glUniform1f(post_time_location, glfwGetTime());
		glUniform1f(post_gamma_location, 1.0f/2.2f);

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
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
		glBindBuffer(GL_ARRAY_BUFFER, quad_uv_buffer);
		glVertexAttribPointer(
			1,                                // Атрибут. Здесь необязательно указывать 1, но главное, чтобы это значение совпадало с layout в шейдере..
			2,                                // Размер
			GL_FLOAT,                         // Тип
			GL_FALSE,                         // Нормализован?
			0,                                // Шаг
			(void*)0                          // Смещение
		);
		
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Сбрасываем буферы
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}