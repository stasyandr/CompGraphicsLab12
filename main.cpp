// Рисует кубик в клеточку

#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include<math.h>
// В C и C++ есть оператор #, который позволяет превращать параметры макроса в строку
#define TO_STRING(x) #x
struct Vertex3D
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

struct myTexture
{
	GLfloat x;
	GLfloat y;
};
struct VOAVertex
{
	int coord;
	int norm;
	int tex;
};

struct VOAVertex2
{
	Vertex3D coord;
	Vertex3D norm;
	myTexture tex;
};
std::vector<Vertex3D> vert;
std::vector<Vertex3D> normals;
std::vector<myTexture> textures;
std::vector<VOAVertex> edges;


void ReadFile(std::string name)
{
	//обьявляем переменную которая будет ссылкой на файл 
	std::fstream FileObj;
	FileObj.open(name, std::ios::in);
	if (FileObj.is_open())
	{
		//обьявляем переменную для чтения строк текста из файла
		std::string ReadLine;
		//Читаем файл пока он не закончился
		while (!FileObj.eof())
		{
			//получаем 1 строку из файла
			std::getline(FileObj, ReadLine, '\n');
			//получаем 1 символ из строки и по нему определяем что будет читаться в данный момент
			switch (ReadLine[0])
			{
				//значит это координаты 
			case 'v':
				//читаем 2 символ из строки 
				switch (ReadLine[1])
				{
					//координаты точки
				case ' ':
					//выведем чито это точка
					int tek;
					tek = 2;
					int next;
					Vertex3D point;
					for (int i = 0; (next = ReadLine.find(' ', tek)) != std::string::npos; i++) {
						switch (i) {
						case 0:
							point.x = atof(ReadLine.substr(tek, next - tek).c_str());
							break;
						case 1:
							point.y = atof(ReadLine.substr(tek, next - tek).c_str());
						default:
							break;
						}
						tek = next + 1;
					}
					point.z = atof(ReadLine.substr(tek, std::string::npos).c_str());
					vert.push_back(point);
					break;
					//координаты нормали
				case 'n':
					tek = 3;
					Vertex3D normal;
					for (int i = 0; (next = ReadLine.find(' ', tek)) != std::string::npos; i++) {
						switch (i) {
						case 0:
							normal.x = atof(ReadLine.substr(tek, next - tek).c_str());
							break;
						case 1:
							normal.y = atof(ReadLine.substr(tek, next - tek).c_str());
						default:
							break;
						}
						tek = next + 1;
					}
					normal.z = atof(ReadLine.substr(tek, std::string::npos).c_str());
					normals.push_back(normal);
					break;
					//координаты текстуры
				case 't':
					tek = 3;
					myTexture tex;
					next = ReadLine.find(' ', tek);
					tex.x = atof(ReadLine.substr(tek, next - tek).c_str());
					tex.y = atof(ReadLine.substr(next + 1, std::string::npos).c_str());
					textures.push_back(tex);
					break;

				default:
					break;
				}

				break;

			case 'f':
				int tek;
				tek = 2;
				int next;
				VOAVertex edge;
				for (int i = 0; tek != 0; i++) {
					next = ReadLine.find(' ', tek);
					std::string tekedge = ReadLine.substr(tek, next - tek);
					int next2 = tekedge.find('/');
					edge.coord = atoi(tekedge.substr(0, next2).c_str());
					int tek2 = next2 + 1;
					next2 = tekedge.find('/', tek2);
					edge.tex = atoi(tekedge.substr(tek2, next2).c_str());
					edge.norm = atoi(tekedge.substr(next2 + 1, std::string::npos).c_str());
					tek = next + 1;
					edges.push_back(edge);
				}
				break;
			default:
				break;
			}
		}
	}
}


// Переменные с индентификаторами ID
// ID шейдерной программы
GLuint Program;
// ID атрибута
GLint Attrib_vertex;
// ID Vertex Buffer Object
GLuint VBO;
// ID атрибута цвета
GLint vertColor;
// ID атрибута текстурных координат
GLuint colorVBO;

// ID юниформа угла поворота
GLint unifAngle1;
// ID юниформа угла поворота
GLint unifAngle2;
// ID юниформа текстуры
GLint unifTexture;
// ID атрибута текстурных координат
GLint attribTexture;
// ID буфера текстурных координат
GLuint textureVBO;
// ID текстуры
GLint textureHandle;
// SFML текстура
sf::Texture textureData;
// Вершина
struct Vertex
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

float objectRotate1 = 1;
float objectRotate2 = 1;

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
    #version 330 core

    in vec3 vertexPosition;
    uniform float x_angle;
    uniform float y_angle;
	in vec3 vertexColor;
	in vec2 texureCoord;
    out vec3 vPosition;
	out vec3 vertCol;
	out vec2 tCoord; 

    void main() {
        vPosition = vertexPosition;
		tCoord = texureCoord;
		vertCol = vertexColor;
        vec3 position = vertexPosition * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        );
        gl_Position = vec4(position, 1.0);
    }
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
    #version 330 core
    uniform sampler2D textureData;
	in vec3 vPosition;
	in vec3 vertCol;
	in vec2 tCoord; 
    out vec4 color;
    void main() {
        float k = 5;
		float x = vertCol.x * 0.5f + 0.5f;
		float y = vertCol.y * 0.5f + 0.5f;
		float z = vertCol.z * 0.5f + 0.5f;
		vec4 vert_color =  vec4(x,y,z,1.0f);
		//vec4 vert_color =  vec4(1.0f,1.0f,1.0f,1.0f);
		vec4 tex = texture(textureData, tCoord.xy);
        color = vec4(vert_color.r*tex.r,vert_color.g*tex.g,vert_color.b*tex.b,vert_color.a*tex.a);
    }
)";


void Init();
void Draw();
void Release();
void InitTexture();

int main() {
	ReadFile("cube.obj");
	sf::Window window(sf::VideoMode(600, 600), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
	window.setVerticalSyncEnabled(true);

	window.setActive(true);

	// Инициализация glew
	glewInit();

	Init();

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Left)
				{
					objectRotate2 += 0.1f;
				}
				if (event.key.code == sf::Keyboard::Down)
				{
					objectRotate1 -= 0.1f;
				}
				if (event.key.code == sf::Keyboard::Right)
				{
					objectRotate2 -= 0.1f;
				}
				if (event.key.code == sf::Keyboard::Up)
				{
					objectRotate1 += 0.1f;
				}

				if (event.key.code == sf::Keyboard::W)
				{
					objectRotate1 += 0.1f;
					objectRotate2 += 0.1f;
				}

				if (event.key.code == sf::Keyboard::S)
				{
					objectRotate1 -= 0.1f;
					objectRotate2 -= 0.1f;
				}
			}
			else if (event.type == sf::Event::Resized) {
				glViewport(0, 0, event.size.width, event.size.height);
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Draw();

		window.display();
	}

	Release();
	return 0;
}


// Проверка ошибок OpenGL, если есть то вывод в консоль тип ошибки
void checkOpenGLerror() {
	GLenum errCode;
	// Коды ошибок можно смотреть тут
	// https://www.khronos.org/opengl/wiki/OpenGL_Error
	if ((errCode = glGetError()) != GL_NO_ERROR)
		std::cout << "OpenGl error!: " << errCode << std::endl;
}

// Функция печати лога шейдера
void ShaderLog(unsigned int shader)
{
	int infologLen = 0;
	int charsWritten = 0;
	char* infoLog;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 1)
	{
		infoLog = new char[infologLen];
		if (infoLog == NULL)
		{
			std::cout << "ERROR: Could not allocate InfoLog buffer" << std::endl;
			exit(1);
		}
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
		std::cout << "InfoLog: " << infoLog << "\n\n\n";
		delete[] infoLog;
	}
}


void InitVBO()
{
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &colorVBO);
	glGenBuffers(1, &textureVBO);
	Vertex3D * arr1 = new Vertex3D[edges.size()];
	for (int i = 0; i < edges.size(); i++) {
		arr1[i] = vert[edges[i].coord - 1];
	}
	Vertex3D * arr2 = new Vertex3D[edges.size()];
	for (int i = 0; i < edges.size(); i++) {
		arr2[i] = normals[edges[i].norm - 1];
	}
	myTexture * arr3 = new myTexture[edges.size()];
	for (int i = 0; i < edges.size(); i++) {
		arr3[i] = textures[edges[i].tex - 1];
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex3D) * edges.size(), arr1, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex3D) * edges.size(), arr2, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myTexture) * edges.size(), arr3, GL_STATIC_DRAW);
	checkOpenGLerror();
}


void InitShader() {
	// Создаем вершинный шейдер
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	// Передаем исходный код
	glShaderSource(vShader, 1, &VertexShaderSource, NULL);
	// Компилируем шейдер
	glCompileShader(vShader);
	std::cout << "vertex shader \n";
	ShaderLog(vShader);

	// Создаем фрагментный шейдер
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Передаем исходный код
	glShaderSource(fShader, 1, &FragShaderSource, NULL);
	// Компилируем шейдер
	glCompileShader(fShader);
	std::cout << "fragment shader \n";
	ShaderLog(fShader);

	// Создаем программу и прикрепляем шейдеры к ней
	Program = glCreateProgram();
	glAttachShader(Program, vShader);
	glAttachShader(Program, fShader);

	// Линкуем шейдерную программу
	glLinkProgram(Program);
	// Проверяем статус сборки
	int link_ok;
	glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		std::cout << "error attach shaders \n";
		return;
	}

	// Вытягиваем ID атрибута из собранной программы
	Attrib_vertex = glGetAttribLocation(Program, "vertexPosition");
	if (Attrib_vertex == -1)
	{
		std::cout << "could not bind attrib " << "vertexPosition" << std::endl;
		return;
	}


	unifAngle1 = glGetUniformLocation(Program, "x_angle");
	if (unifAngle1 == -1)
	{
		std::cout << "could not bind uniform angle" << std::endl;
		return;
	}

	unifAngle2 = glGetUniformLocation(Program, "y_angle");
	if (unifAngle1 == -1)
	{
		std::cout << "could not bind uniform angle" << std::endl;
		return;
	}

	vertColor = glGetAttribLocation(Program, "vertexColor");
	if (vertColor == -1)
	{
		std::cout << "could not bind attrib " << "vertexColor" << std::endl;
		return;
	}
	attribTexture = glGetAttribLocation(Program, "texureCoord");
	if (attribTexture == -1)
	{
		std::cout << "could not bind attrib texureCoord" << std::endl;
		return;
	}

	unifTexture = glGetUniformLocation(Program, "textureData");
	if (unifTexture == -1)
	{
		std::cout << "could not bind uniform textureData" << std::endl;
		return;
	}
	checkOpenGLerror();
}

void Init() {
	InitShader();
	InitVBO();
	InitTexture();
	// Включаем проверку глубины
	glEnable(GL_DEPTH_TEST);
}


void Draw() {
	// Устанавливаем шейдерную программу текущей
	glUseProgram(Program);
	glUniform1f(unifAngle1, objectRotate1);
	glUniform1f(unifAngle2, objectRotate2);

	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&textureData);
	glUniform1i(unifTexture, 0);

	glEnableVertexAttribArray(Attrib_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glEnableVertexAttribArray(vertColor);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glVertexAttribPointer(vertColor, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glEnableVertexAttribArray(attribTexture);
	glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
	glVertexAttribPointer(attribTexture, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Передаем данные на видеокарту(рисуем)
	glDrawArrays(GL_TRIANGLES, 0, edges.size());
	// Отключаем массив атрибутов
	glDisableVertexAttribArray(Attrib_vertex);
	glDisableVertexAttribArray(vertColor);
	glDisableVertexAttribArray(attribTexture);
	// Отключаем шейдерную программу
	glUseProgram(0);
	checkOpenGLerror();
}

void InitTexture()
{
	const char* filename = "putin.jpg";
	// Загружаем текстуру из файла
	if (!textureData.loadFromFile(filename))
	{
		// Не вышло загрузить картинку
		return;
	}
	// Теперь получаем openGL дескриптор текстуры
	textureHandle = textureData.getNativeHandle();
}

// Освобождение шейдеров
void ReleaseShader() {
	// Передавая ноль, мы отключаем шейдрную программу
	glUseProgram(0);
	// Удаляем шейдерную программу
	glDeleteProgram(Program);
}

// Освобождение буфера
void ReleaseVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO);
}

void Release() {
	ReleaseShader();
	ReleaseVBO();
}
