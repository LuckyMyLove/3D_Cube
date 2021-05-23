#include <iostream>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//Headers
#include <GL/glew.h>
#include <SFML/Window.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <fstream>

using namespace std;



//Shaders codes
const GLchar* vertexSource = R"glsl(
#version 150 core
in vec3 position;
in vec3 color;
out vec3 Color;
in vec2 aTexCoord;
out vec2 TexCoord;
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 proj;

void main(){
	TexCoord = aTexCoord;
	Color = color;
	gl_Position = proj * view * model * vec4(position, 1.0);
}
)glsl";

const GLchar* fragmentSource = R"glsl(
#version 150 core
in vec3 Color;
out vec4 outColor;
in vec2 TexCoord;
	uniform sampler2D texture0;
	uniform sampler2D texture1;

void main()
{
//outColor = mix(texture(texture0, TexCoord), texture(texture1, TexCoord), 0.5);
outColor=texture(texture0, TexCoord);
//outColor = vec4(Color, 1.0);
}
)glsl";

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -3.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;

float sep = 0.05;
int mode = 0;

float rotation = 0;
int lastX, lastY;
double yaw = -90; //rotation on X axis
double pitch = 0; //rotation on Y axis

bool LoadModelOBJNormalsCoord(int& points_, int points_for_mono_[], const char* filename, int buffer)
{
	int vert_num = 0;
	int triangles = 0;
	int normals = 0;
	int coord_num = 0;

	std::ifstream myReadFile;
	myReadFile.open(filename);
	std::string output;
	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			myReadFile >> output;
			if (output == "v") vert_num++;
			if (output == "f") triangles++;
			if (output == "vn") normals++;
			if (output == "vt") coord_num++;
		}
	}

	myReadFile.close();
	myReadFile.open(filename);


	float** vert;
	vert = new float* [vert_num]; //przydzielenie pamięci na w wierszy

	for (int i = 0; i < vert_num; i++)
		vert[i] = new float[3];


	int** trian;
	trian = new int* [triangles]; //przydzielenie pamięci na w wierszy

	for (int i = 0; i < triangles; i++)
		trian[i] = new int[9];

	float** norm;
	norm = new float* [normals]; //przydzielenie pamięci na w wierszy

	for (int i = 0; i < normals; i++)
		norm[i] = new float[3];

	float** coord;
	coord = new float* [coord_num]; //przydzielenie pamięci na w wierszy

	for (int i = 0; i < coord_num; i++)
		coord[i] = new float[2];

	int licz_vert = 0;
	int licz_triang = 0;
	int licz_normals = 0;
	int licz_coord = 0;

	int obj_number = 0; //numer aktualnego obiektu
	int f_lines_per_object[5]; //zmienna pomocnicza do liczenia ilosci linijek f dla kazdego obiektu osobno

	f_lines_per_object[0] = 0;

	while (!myReadFile.eof()) {
		output = "";
		myReadFile >> output;
		if (output == "vn") { myReadFile >> norm[licz_normals][0]; myReadFile >> norm[licz_normals][1]; myReadFile >> norm[licz_normals][2]; licz_normals++; }
		if (output == "v") { myReadFile >> vert[licz_vert][0]; myReadFile >> vert[licz_vert][1]; myReadFile >> vert[licz_vert][2]; licz_vert++; }
		if (output == "vt") { myReadFile >> coord[licz_coord][0]; myReadFile >> coord[licz_coord][1]; licz_coord++; }

		if (output == "f") {

			for (int i = 0; i < 9; i += 3)
			{
				std::string s;
				myReadFile >> s;
				std::stringstream ss(s);

				std::vector <std::string> el;
				std::string item;


				while (getline(ss, item, '/')) {
					el.push_back(item);
				}
				trian[licz_triang][i] = std::stoi(el[0]);
				trian[licz_triang][i + 1] = std::stoi(el[1]);
				trian[licz_triang][i + 2] = std::stoi(el[2]);


			}
			licz_triang++;
		}

		if (output == "o") obj_number++;

		if (obj_number > 0) {
			f_lines_per_object[obj_number] = licz_triang - f_lines_per_object[obj_number - 1] - 1; //obliczamy ile linijek f przypada na jeden obiekt
			points_for_mono_[obj_number] = f_lines_per_object[obj_number] * 3; //wlasciwa ilosc trojkatow
		}

	}


	GLfloat* vertices = new GLfloat[triangles * 24];

	int vert_current = 0;

	for (int i = 0; i < triangles; i++)
	{
		vertices[vert_current] = vert[trian[i][0] - 1][0];
		vertices[vert_current + 1] = vert[trian[i][0] - 1][1];
		vertices[vert_current + 2] = vert[trian[i][0] - 1][2];
		vertices[vert_current + 3] = norm[trian[i][2] - 1][0];
		vertices[vert_current + 4] = norm[trian[i][2] - 1][1];
		vertices[vert_current + 5] = norm[trian[i][2] - 1][2];
		vertices[vert_current + 6] = coord[trian[i][1] - 1][0];
		vertices[vert_current + 7] = coord[trian[i][1] - 1][1];

		vertices[vert_current + 8] = vert[trian[i][3] - 1][0];
		vertices[vert_current + 9] = vert[trian[i][3] - 1][1];
		vertices[vert_current + 10] = vert[trian[i][3] - 1][2];
		vertices[vert_current + 11] = norm[trian[i][5] - 1][0];
		vertices[vert_current + 12] = norm[trian[i][5] - 1][1];
		vertices[vert_current + 13] = norm[trian[i][5] - 1][2];
		vertices[vert_current + 14] = coord[trian[i][4] - 1][0];
		vertices[vert_current + 15] = coord[trian[i][4] - 1][1];

		vertices[vert_current + 16] = vert[trian[i][6] - 1][0];
		vertices[vert_current + 17] = vert[trian[i][6] - 1][1];
		vertices[vert_current + 18] = vert[trian[i][6] - 1][2];
		vertices[vert_current + 19] = norm[trian[i][8] - 1][0];
		vertices[vert_current + 20] = norm[trian[i][8] - 1][1];
		vertices[vert_current + 21] = norm[trian[i][8] - 1][2];
		vertices[vert_current + 22] = coord[trian[i][7] - 1][0];
		vertices[vert_current + 23] = coord[trian[i][7] - 1][1];

		vert_current += 24;
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * triangles * 24, vertices, GL_STATIC_DRAW);

	points_ = triangles * 3;

	delete vertices;




	for (int i = 0; i < vert_num; i++)
		delete[] vert[i];
	delete[] vert;

	for (int i = 0; i < triangles; i++)
		delete[] trian[i];
	delete[] trian;

	for (int i = 0; i < normals; i++)
		delete[] norm[i];
	delete[] norm;

	for (int i = 0; i < coord_num; i++)
		delete[] coord[i];
	delete[] coord;

	return 0;
}

void generateCube(int buffer) {
	int vertex = 36;

	//Tabica wierzchołków sześcianu / Table of vertices of the cube
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f
	};

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) *vertex * 8, vertices, GL_STATIC_DRAW);
}

void setCameraKeyboard(GLint _uniView, float _time) {
	float cameraSpeed = 0.000002f * _time;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
		cameraPos += cameraSpeed * cameraFront;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	glm::mat4 view;
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glUniformMatrix4fv(_uniView, 1, GL_FALSE, glm::value_ptr(view));
}

void setCameraMouse(GLint view_, float elapsedTime_, const sf::Window& window_) {

	// Download currect location of the mouse / Pobranie aktualnej lokalizacji kursora w oknie
	sf::Vector2i localPosition = sf::Mouse::getPosition(window_);

	// Vector to set - Wektor do ustawiania
	sf::Vector2i position;
	bool relocation = false;

	// Free move of camera by mouse - Swoboda ruchu dla kursora myszy
	if (localPosition.x <= 0) {
		position.x = window_.getSize().x - 1;
		position.y = localPosition.y;
		relocation = true;
	}
	if (localPosition.x >= window_.getSize().x - 1) {
		position.x = 0;
		position.y = localPosition.y;
		relocation = true;
	}
	if (localPosition.y <= 0) {
		position.x = window_.getSize().y - 1;
		position.y = localPosition.x;
		relocation = true;
	}
	if (localPosition.y >= window_.getSize().y - 1) {
		position.x = window_.getSize().x - 1;
		position.y = localPosition.x;
		relocation = true;
	}
	// relocatioan == true is moving mouse to new position / relocatioan == true przerzuca kursor na nową pozycję
	if (relocation) {
		sf::Mouse::setPosition(position, window_);
		firstMouse = true;
	}
	localPosition = sf::Mouse::getPosition(window_);


	// Protection against extreme positions / Zabezpieczenie przed odjęciem skrajnych położeń
	if (firstMouse)
	{
		lastX = localPosition.x;
		lastY = localPosition.y;
		firstMouse = false;
	}

	//Determining the position change and storing the last position / Wyznaczenie zmiany położenia i zapamiętanie ostatniej pozycji
	double xoffset = localPosition.x - lastX;
	double yoffset = localPosition.y - lastY;
	lastX = localPosition.x;
	lastY = localPosition.y;

	double sensitivity = 0.1f;
	double cameraSpeed = 0.00005f * elapsedTime_;


	//Update camera angles / Aktualizacja kątów ustawienia kamery:
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset * cameraSpeed;
	pitch -= yoffset * cameraSpeed;

	//Imposition of constraints on the maximum values of angles and determination of new values for the camera tilt vector
	//Nałożenie ograniczeń na maksymalne wartości kątów i wyznaczenie nowych wartości wektora pochylenia kamery
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);



	//Creating current matrix of view / Tworzenie aktualnej macierzy widoku:
	glm::mat4 view;
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glUniformMatrix4fv(view_, 1, GL_FALSE, glm::value_ptr(view));

}

void StereoProjection(GLuint shaderProgram_, float _left, float _right, float _bottom, float _top, float _near, float _far, float _zero_plane, float _dist, float _eye)
{
	float   _dx = _right - _left;
	float   _dy = _top - _bottom;

	float   _xmid = (_right + _left) / 2.0;
	float   _ymid = (_top + _bottom) / 2.0;

	float   _clip_near = _dist + _zero_plane - _near;
	float   _clip_far = _dist + _zero_plane - _far;

	float  _n_over_d = _clip_near / _dist;

	float   _topw = _n_over_d * _dy / 2.0;
	float   _bottomw = -_topw;
	float   _rightw = _n_over_d * (_dx / 2.0 - _eye);
	float   _leftw = _n_over_d * (-_dx / 2.0 - _eye);

	// Create a fustrum, and shift it off axis
	glm::mat4 proj = glm::frustum(_leftw, _rightw, _bottomw, _topw, _clip_near, _clip_far);

	proj = glm::translate(proj, glm::vec3(-_xmid - _eye, -_ymid, 0));

	GLint uniProj = glGetUniformLocation(shaderProgram_, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
}

string checkShaders(const GLuint& shader) {
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	string info{};

	if (status == 1) {
		info.append("OK\n");
	}
	else {
		info.append("FAILED\n");
	}

	GLchar* log = new GLchar[1024];
	glGetShaderInfoLog(shader, 1024, nullptr, log);
	info.append(log);
	delete[] log;
	return info;
}


int main() {
	sf::ContextSettings settings;
	settings.depthBits = 36;
	settings.stencilBits = 8;


	// Rendering window / Okno renderingu
	sf::Window window(sf::VideoMode(800, 800, 32), "OpenGL", sf::Style::Titlebar | sf::Style::Close, settings);

	window.setMouseCursorVisible(false); //Hidding mouse cursor / Ukrycie kursora myszy
	window.setMouseCursorGrabbed(true); //Catching mouse curson in the window / Przechwycanie kursora myszy w oknie


	// GLEW initialisation / Inicjalizacja GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	// Creating VAD (Vertex Array Object) / Utworzenie VAO 
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Creating VBO (Vertex Buffer Object) and copying vertex data into him
	// Utworzenie VBO i skopiowanie do niego danych wierzchołkowych
	GLuint vbo;
	glGenBuffers(1, &vbo);

	generateCube(vbo);

	int points = 0;
	int points_for_mono[5];
	points_for_mono[0] = 0;

	LoadModelOBJNormalsCoord(points, points_for_mono, "objects.obj", vbo);

	for (int i = 0; i < 5; i++) {
		cout << "Obiekt nr. triangels number" << i+1 << " : " << points_for_mono[i] << endl;
	}
	// Creation and compilation of a vertex shader
	// Utworzenie i skompilowanie shadera wierzchołków
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	cout << "Vertex Shader compilation status: " << checkShaders(vertexShader);

	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);

	// Creation and compilation of a fragment shader
	// Utworzenie i skompilowanie shadera fragmentów
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	cout << "Fragment Shader compilation status: " << checkShaders(fragmentShader);

	// Linking both shaders into one common program
	// Zlinkowanie obu shaderów w jeden wspólny program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Specifying format of vertex data
	// Specifikacja formatu danych wierzchołkowych
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);

	//GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	//glEnableVertexAttribArray(colAttrib);
	//glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	GLint TexCoord = glGetAttribLocation(shaderProgram, "aTexCoord");
	glEnableVertexAttribArray(TexCoord);
	glVertexAttribPointer(TexCoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));


	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	//Sending to the shader / Wysłanie do shadera
	GLint uniTrans = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));

	GLint uniView = glGetUniformLocation(shaderProgram, "view");

	glm::mat4 proj = glm::perspective(glm::radians(60.0f), 800.0f / 800.0f, 0.06f, 100.0f);
	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");

	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	GLint primitive = GL_TRIANGLES;
	GLint mouse_x = 0;
	GLint mouse_y = 0;

	//z-bufor
	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, 800, 800);



	unsigned int texture0;

	//Generating textures / Generuje tekstury
	glGenTextures(1, &texture0); 

	//Setting texture as the current one / Ustawienie tekstury jako bierzącej
	glBindTexture(GL_TEXTURE_2D, texture0); 

	// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Load image, create texture and generate mipmaps	
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); //tell stb_image.h to flip loaded textrue's on the y-axis
	unsigned char* data = stbi_load("table.bmp", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);



	unsigned int texture1;

	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("metal.bmp", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);


	unsigned int texture2;

	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("obraz.bmp", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);


	unsigned int texture3;

	glGenTextures(1, &texture3);
	glBindTexture(GL_TEXTURE_2D, texture3);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("crate.bmp", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glUniform1i(glGetUniformLocation(shaderProgram, "texture0"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 1);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 2);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture3"), 3);



	// Running of the main look / Rozpoczęcie pętli zdarzeń
	bool running = true;
	GLint mouse_vertical = 0;

	window.setFramerateLimit(60);
	sf::Clock clock;
	sf::Time time;

	int counter = 0;
	

	while (running) {

		time = clock.getElapsedTime();
		clock.restart();

		counter++;
		float fpsy = 1000000 / time.asMicroseconds();
		if (counter > fpsy) {
			window.setTitle(std::to_string(fpsy));
			counter = 0;
		}


		sf::Event windowEvent;
		while (window.pollEvent(windowEvent)) {
			switch (windowEvent.type) {
			case sf::Event::Closed:
				running = false;
				break;

			case sf::Event::KeyPressed:
				switch (windowEvent.key.code) {
				case sf::Keyboard::Escape:
					running = false;
					break;

				case sf::Keyboard::Num0:
					primitive = GL_POINTS;
					break;
				case sf::Keyboard::Num1:
					primitive = GL_TRIANGLES;
					break;
				case sf::Keyboard::Num2:
					primitive = GL_TRIANGLE_STRIP;
					break;
				case sf::Keyboard::Num3:
					primitive = GL_TRIANGLE_FAN;
					break;
				case sf::Keyboard::Num4:
					primitive = GL_LINES;
					break;
				case sf::Keyboard::Num5:
					primitive = GL_LINE_STRIP;
					break;
				case sf::Keyboard::Num6:
					primitive = GL_LINE_LOOP;
					break;
				case sf::Keyboard::Num7:
					primitive = GL_QUADS;
					break;
				case sf::Keyboard::Num8:
					primitive = GL_QUAD_STRIP;
					break;
				case sf::Keyboard::Num9:
					primitive = GL_POLYGON;
					break;		
				case sf::Keyboard::W:
					mode = 0;
					break;	
				case sf::Keyboard::A:
					mode = 1;
					break;
				case sf::Keyboard::D:
					mode = 2;
					break;
				case sf::Keyboard::Q:
					sep += 0.01;
					break;
				case sf::Keyboard::E:
					sep -= 0.01;
					break;
				}


			case sf::Event::MouseMoved:
				setCameraMouse(uniView, time.asMicroseconds(), window);
				break;
			}
		}

		setCameraKeyboard(uniView, time.asMicroseconds());


		// Giving the scene a black colour / Nadanie scenie koloru czarnego
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture0);
		glDrawArrays(GL_TRIANGLES, 0, 12);

		glBindTexture(GL_TEXTURE_2D, texture1);
		glDrawArrays(GL_TRIANGLES, 12, 12);



		//Changing visual mode / Zmiana trybu wyświetlania
		switch (mode) {

		case 0: //Mono
			glActiveTexture(GL_TEXTURE0);
			glViewport(0, 0, window.getSize().x, window.getSize().y);

			glBindTexture(GL_TEXTURE_2D, texture0);
			glDrawArrays(primitive, points_for_mono[0], points_for_mono[1]);

			glBindTexture(GL_TEXTURE_2D, texture1);
			glDrawArrays(primitive, points_for_mono[1], points_for_mono[2]);

			glBindTexture(GL_TEXTURE_2D, texture2);
			glDrawArrays(primitive, (points_for_mono[1] + points_for_mono[2]), points_for_mono[3]);

			glBindTexture(GL_TEXTURE_2D, texture3);
			glDrawArrays(primitive, (points_for_mono[1] + points_for_mono[2] + points_for_mono[3]), points_for_mono[4]);
			break;
			
		case 1: //Site by site
			glViewport(0, 0, window.getSize().x/2, window.getSize().y);
			StereoProjection(shaderProgram, -6, 6, -4.8, 4.8, 12.99, -100, 0, 13, -0.05);
			glDrawArrays(primitive, 0, points);

			glViewport(window.getSize().x/2, 0, window.getSize().x/2, window.getSize().y);
			StereoProjection(shaderProgram, -6, 6, -4.8, 4.8, 12.99, -100, 0, 13, 0.05);
			glDrawArrays(primitive, 0, points);
			break;

		case 2: //Anaglyph mode  
			glViewport(0, 0, window.getSize().x, window.getSize().y);
			glDrawBuffer(GL_BACK_LEFT);
			StereoProjection(shaderProgram, -6, 6, -4.8, 4.8, 12.99, -100, 0, 13, -sep);
			glColorMask(true, false, false, false);
			glDrawArrays(primitive, 0, points);

			glClear(GL_DEPTH_BUFFER_BIT);

			glDrawBuffer(GL_BACK_RIGHT);
			StereoProjection(shaderProgram,-6, 6, -4.8, 4.8, 12.99, -100, 0, 13, sep);
			glColorMask(false, false, true, false);
			glDrawArrays(primitive, 0, points);
			glColorMask(true, true, true, true);
			break;
		}

		// Rear/front buffer replacement / Wymiana buforów tylni/przedni
		window.display();
	}

	// Program deletion and buffer clearing
	// Kasowanie programu i czyszczenie buforów
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	// Close the rendering window
	// Zamknięcie okna renderingu
	window.close();
	return 0;
}