#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cctype>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

const double PI = 3.14159265358979323846;

// vertex shader source code
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(aPos, 1.0);\n"
"}\0";

// fragment shader source code
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 inputColor;\n"
"void main()\n"
"{\n"
"FragColor = inputColor;\n"
"}\0";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {

	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}

void airfoilGenerator(std::string nacaNumber, unsigned int airfoilVBO, std::vector<float>& airfoilVertices, unsigned int camberVBO, std::vector<float>& camberVertices, unsigned int chordVBO, std::vector<float>& chordVertices) {

	if (nacaNumber.length() == 4 && std::isdigit(nacaNumber[0]) && std::isdigit(nacaNumber[1]) && std::isdigit(nacaNumber[2]) && std::isdigit(nacaNumber[3])) {
		std::cout << "NACA 4 series: " << nacaNumber << std::endl;

		int divisions = 1000;

		std::vector<float> airfoilVerticesUpper((divisions + 1) * 3);
		std::vector<float> airfoilVerticesLower((divisions + 1) * 3);

		double m = static_cast<double>(nacaNumber[0] - '0') / 100;
		double p = static_cast<double>(nacaNumber[1] - '0') / 10;
		double t = std::stof(nacaNumber.substr(2, 2)) / 100;

		camberVertices.clear();

		std::cout << "m: " << m << " p: " << p << " t: " << t << std::endl;

		for (int i = 0; i <= divisions; i++) {
			double x = (1 - cos(PI * (double(i) / double(divisions)))) / 2;
			//std::cout << "Iteration Number: " << i << " Size: " << divisions << " Position: " << x << std::endl;

			double y_t = 5 * t * (0.2969 * sqrt(x) - 0.1260 * x - 0.3516 * pow(x, 2) + 0.2843 * pow(x, 3) - 0.1036 * pow(x, 4));

			double y_c = 0.0;
			double dy_c__dx = 0.0;

			if (p == 0.0 || m == 0.0) {
				y_c = 0.0;
				dy_c__dx = 0.0;
			}
			else if (x <= p) {
				y_c = (m / pow(p, 2)) * (2 * p * x - pow(x, 2));
				dy_c__dx = ((2 * m) / pow(p, 2)) * (p - x);
			}
			else {
				y_c = (m / pow(1 - p, 2)) * ((1 - 2 * p) + 2 * p * x - pow(x, 2));
				dy_c__dx = ((2 * m) / pow(1 - p, 2)) * (p - x);
			}

			camberVertices.push_back(x - 0.5);
			camberVertices.push_back(y_c);
			camberVertices.push_back(0.0);

			double theta = atan(dy_c__dx);

			double x_u = x - y_t * sin(theta);
			double x_l = x + y_t * sin(theta);
			double y_u = y_c + y_t * cos(theta);
			double y_l = y_c - y_t * cos(theta);

			airfoilVerticesUpper.at(i * 3) = float(x_u - 0.5);
			airfoilVerticesUpper.at(i * 3 + 1) = float(y_u);
			airfoilVerticesUpper.at(i * 3 + 2) = 0.0;

			airfoilVerticesLower.at(((divisions + 1) * 3) - i * 3 - 3) = float(x_l - 0.5);
			airfoilVerticesLower.at(((divisions + 1) * 3) - i * 3 - 2) = float(y_l);
			airfoilVerticesLower.at(((divisions + 1) * 3) - i * 3 - 1) = 0.0;
		}

		//std::cout << "Loop Passed" << std::endl;

		airfoilVertices.clear();

		airfoilVertices.insert(airfoilVertices.end(), airfoilVerticesUpper.begin(), airfoilVerticesUpper.end());
		airfoilVertices.insert(airfoilVertices.end(), airfoilVerticesLower.begin(), airfoilVerticesLower.end());


		chordVertices.clear();

		chordVertices.push_back(airfoilVerticesUpper[0]);
		chordVertices.push_back(airfoilVerticesUpper[1]);
		chordVertices.push_back(airfoilVerticesUpper[2]);

		chordVertices.push_back(airfoilVerticesUpper[airfoilVerticesUpper.size() - 3]);
		chordVertices.push_back(airfoilVerticesUpper[airfoilVerticesUpper.size() - 2]);
		chordVertices.push_back(airfoilVerticesUpper[airfoilVerticesUpper.size() - 1]);

		//std::cout << "vertices created" << std::endl;

		glBindBuffer(GL_ARRAY_BUFFER, airfoilVBO);
		glBufferData(GL_ARRAY_BUFFER, airfoilVertices.size() * sizeof(float), airfoilVertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, chordVBO);
		glBufferData(GL_ARRAY_BUFFER, chordVertices.size() * sizeof(float), chordVertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, camberVBO);
		glBufferData(GL_ARRAY_BUFFER, camberVertices.size() * sizeof(float), camberVertices.data(), GL_STATIC_DRAW);

		//std::cout << "Buffers Bound" << std::endl;
	}
	else {
		std::cout << "Enter a valid NACA airfoil number" << std::endl;
	}
};

int main() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(800, 800, "OpenGL_Testing", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, 800, 800);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Rendering Initialization

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Airfoil VAO
	unsigned int airfoilVAO;
	glGenVertexArrays(1, &airfoilVAO);

	glBindVertexArray(airfoilVAO);

	std::vector<float> airfoilVertices = {
		  0.5f, -0.5f, 0.0f,
		 -0.5f, -0.5f, 0.0f,
		  0.0f,  0.5f, 0.0f
	};

	unsigned int airfoilVBO;
	glGenBuffers(1, &airfoilVBO);

	glBindBuffer(GL_ARRAY_BUFFER, airfoilVBO);
	glBufferData(GL_ARRAY_BUFFER, airfoilVertices.size() * sizeof(float), airfoilVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Camber Line VAO
	unsigned int camberVAO;
	glGenVertexArrays(1, &camberVAO);

	glBindVertexArray(camberVAO);

	std::vector<float> camberVertices = {
		  0.5f,  0.5f, 0.0f,
		 -0.5f,  0.5f, 0.0f,
		  0.0f, -0.5f, 0.0f
	};

	unsigned int camberVBO;
	glGenBuffers(1, &camberVBO);

	glBindBuffer(GL_ARRAY_BUFFER, camberVBO);
	glBufferData(GL_ARRAY_BUFFER, camberVertices.size() * sizeof(float), camberVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Grid VAO
	unsigned int gridVAO;
	glGenVertexArrays(1, &gridVAO);

	glBindVertexArray(gridVAO);

	std::vector<float> gridVertices = {
		 1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,

		 1.0f,  0.75f, 0.0f,
		-1.0f,  0.75f, 0.0f,

		 1.0f,  0.5f, 0.0f,
		-1.0f,  0.5f, 0.0f,

		 1.0f,  0.25f, 0.0f,
		-1.0f,  0.25f, 0.0f,

		 1.0f, -0.25f, 0.0f,
		-1.0f, -0.25f, 0.0f,

		 1.0f, -0.5f, 0.0f,
		-1.0f, -0.5f, 0.0f,

		 1.0f, -0.75f, 0.0f,
		-1.0f, -0.75f, 0.0f,

		 1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,

		 1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,

		 0.75f,  1.0f, 0.0f,
		 0.75f, -1.0f, 0.0f,
		  
		 0.5f,  1.0f, 0.0f,
		 0.5f, -1.0f, 0.0f,

		 0.25f,  1.0f, 0.0f,
		 0.25f, -1.0f, 0.0f,

		-0.25f,  1.0f, 0.0f,
		-0.25f, -1.0f, 0.0f,

		-0.5f,  1.0f, 0.0f,
		-0.5f, -1.0f, 0.0f,

		-0.75f,  1.0f, 0.0f,
		-0.75f, -1.0f, 0.0f,

		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
	};

	unsigned int gridVBO;
	glGenBuffers(1, &gridVBO);

	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Axes VAO
	unsigned int axesVAO;
	glGenVertexArrays(1, &axesVAO);

	glBindVertexArray(axesVAO);

	std::vector<float> axesVertices = {
		 0.0f,  1.0f, 0.0f,
		 0.0f, -1.0f, 0.0f,
		 1.0f,  0.0f, 0.0f,
		-1.0f,  0.0f, 0.0f,
	};

	unsigned int axesVBO;
	glGenBuffers(1, &axesVBO);

	glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
	glBufferData(GL_ARRAY_BUFFER, axesVertices.size() * sizeof(float), axesVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Chord Line VAO
	unsigned int chordVAO;
	glGenVertexArrays(1, &chordVAO);

	glBindVertexArray(chordVAO);

	std::vector<float> chordVertices = {
		  0.25f,  0.25f, 0.0f,
		 -0.25f,  0.25f, 0.0f,
		  0.0f, -0.25f, 0.0f
	};

	unsigned int chordVBO;
	glGenBuffers(1, &chordVBO);

	glBindBuffer(GL_ARRAY_BUFFER, chordVBO);
	glBufferData(GL_ARRAY_BUFFER, chordVertices.size() * sizeof(float), chordVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Vertex Shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int vertexSuccess;
	char vertexInfoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);

	if (!vertexSuccess) {
		glGetShaderInfoLog(vertexShader, 512, NULL, vertexInfoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << vertexInfoLog << std::endl;
	}

	// Fragment Shader 
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	int fragmentSuccess;
	char fragmentInfoLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentSuccess);

	if (!fragmentSuccess) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, fragmentInfoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << fragmentInfoLog << std::endl;
	}

	// Shader Program
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	int shaderSuccess;
	char shaderInfoLog[512];
	glGetProgramiv(shaderProgram, GL_COMPILE_STATUS, &shaderSuccess);

	if (!shaderSuccess) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, shaderInfoLog);
		std::cout << "ERROR::PROGRAM::LINKING::COMPILATION_FAILED\n" << shaderInfoLog << std::endl;
	}

	glUseProgram(shaderProgram);


	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Ui stuff with ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsClassic();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");



	while (!glfwWindowShouldClose(window)) {

		// inputs
		processInput(window);

		// Clear Commands
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Activate Shader Program
		glUseProgram(shaderProgram);

		// Drawing commands
		int inputColorLocation = glGetUniformLocation(shaderProgram, "inputColor");

		glUniform4f(inputColorLocation, 0.5f, 0.5f, 0.5f, 1.0f);
		glBindVertexArray(gridVAO);
		glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);

		glUniform4f(inputColorLocation, 0.8f, 0.8f, 0.0f, 1.0f);
		glBindVertexArray(axesVAO);
		glDrawArrays(GL_LINES, 0, axesVertices.size() / 3);

		glUniform4f(inputColorLocation, 1.0f, 0.0f, 0.0f, 1.0f);
		glBindVertexArray(camberVAO);
		glDrawArrays(GL_LINE_STRIP, 0, camberVertices.size() / 3);

		glUniform4f(inputColorLocation, 0.0f, 0.0f, 1.0f, 1.0f);
		glBindVertexArray(chordVAO);
		glDrawArrays(GL_LINE_STRIP, 0, chordVertices.size() / 3);

		glUniform4f(inputColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
		glBindVertexArray(airfoilVAO);
		glDrawArrays(GL_LINE_LOOP, 0, airfoilVertices.size() / 3);

		glBindVertexArray(0);

		// UI
		static char inputBuffer[5] = "";
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowSize(ImVec2(130, 90));
		ImGui::Begin("NACA Number", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowPadding = ImVec2(8, 8);
		style.FramePadding = ImVec2(4, 4);
		style.ItemSpacing = ImVec2(8, 8);

		ImGui::InputText(" ", inputBuffer, IM_ARRAYSIZE(inputBuffer));
		if (ImGui::Button("Render Airfoil")) {
			airfoilGenerator(inputBuffer, airfoilVBO, airfoilVertices, camberVBO, camberVertices, chordVBO, chordVertices);
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// checking and calling events plus swapping the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}