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

GLuint attribNormals;
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

// ID Transform struct
GLint Unif_transform_model;
GLint Unif_transform_viewProjection;
GLint Unif_transform_normal;
GLint Unif_transform_viewPosition;
// ID Material struct
GLint Unif_material_emission;
GLint Unif_material_ambient;
GLint Unif_material_diffuse;
GLint Unif_material_specular;
GLint Unif_material_shininess;
// ID LigtPoint struct
GLint Unif_light_ambient;
GLint Unif_light_diffuse;
GLint Unif_light_specular;
GLint Unif_light_attenuation;
GLint Unif_light_position;

GLint Unif_trans;

float xyz[3] = { 0.5f, 0.5f, 0.0f };

// Вершина
struct Vertex
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

float objectRotate1 = 1;
float objectRotate2 = 1;




struct Transform
{
	float model[4][4] =
	{
		{1.0, 0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{0.0, 0.0, 0.0, 1.0},
	};
	float viewProjection[4][4] =
	{
		{1.0, 0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{0.0, 0.0, 0.0, 1.0},
	};
	float normal[3][3] =
	{
		{1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, 0.0, 1.0},
	};
	float viewPosition[3] = { -1.0f, -1.0f, -1.0f };
} transform;

struct Material
{
	float ambient[4] = { 0.1f, 0.18725f, 0.1745f, 1.0f };
	float emission[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float diffuse[4] = { 0.396f, 0.74151f, 0.69102f, 1.0f };
	float specular[4] = { 0.297254f, 0.30829f, 0.306678f, 1.0f };
	float shininess = 0.128f;
} material;

struct PointLight
{
	float ambient[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
	float diffuse[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float specular[4] = { 0.7f, 0.7f, 0.7f, 0.7f };
	float attenuation[3] = { 1.0f, 0.0f, 0.0f };
	float position[3] = { -1.5f, -1.5f, -1.5f };
} light;



// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
    #version 330 core

    uniform vec3 xyz;

    in vec3 position;
	in vec3 normal;

	uniform struct Transform {
		mat4 model;
		mat4 viewProjection;
		mat3 normal;
		vec3 viewPosition;
	} transform;

	uniform struct PointLight {
		vec3 position;
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		vec3 attenuation;
	} light;

	out Vertex {
		//vec2 texcoord;
		vec3 normal;
		vec3 lightDir;
		vec3 viewDir;
		float distance;
	} Vert;

    void main() {
		float x_angle = xyz[0];
        float y_angle = xyz[1];
		float z_angle = xyz[2];
        
        mat3 rotate = mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        ) * mat3(
			cos(z_angle), -sin(z_angle), 0,
			sin(z_angle), cos(z_angle), 0,
			0, 0, 1
		);

		vec3 vertex = position * rotate;
		vec3 lightDir = light.position - vertex;
		gl_Position = vec4(vertex, 1.0);
		Vert.normal = normal * rotate;
		Vert.lightDir = lightDir;
		Vert.viewDir = transform.viewPosition - vertex;
		Vert.distance = length(lightDir);
}
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
	#version 330 core


    in Vertex {
		//vec2 texcoord;
		vec3 normal;
		vec3 lightDir;
		vec3 viewDir;
		float distance;
	} Vert;

	uniform struct PointLight {
		vec3 position;
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		vec3 attenuation;
	} light;

	uniform struct Material {
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		vec4 emission;
		float shininess;
	} material;

	out vec4 color;

    void main() {

	int lightingtype = 3;
    if(lightingtype == 1 )
{
       vec3 normal = normalize(Vert.normal);
	   vec3 lightDir = normalize(Vert.lightDir);
	   float diff = 0.2 + max(dot(normal, lightDir), 0.0);
       if(diff < 0.4)
        color = material.diffuse * 0.3;
       else if(diff < 0.7)
        color = material.diffuse;
       else
        color = material.diffuse * 1.3;
}
if(lightingtype == 2 )	
    {    vec3 normal = normalize(Vert.normal);
	    vec3 lightDir = normalize(Vert.lightDir);
	    vec3 viewDir = normalize(Vert.viewDir);
        float attenuation = 1.0/(light.attenuation[0] + light.attenuation[1] * Vert.distance + light.attenuation[2] * Vert.distance * Vert.distance); 	
        color = material.emission;
        color += material.ambient * light.ambient * attenuation;
        float Ndot = max(dot(normal,lightDir),0.0);
        color += material.diffuse * light.diffuse * Ndot* attenuation;
        float RdotVpow = max(pow(dot(reflect (-lightDir, normal), viewDir), material.shininess),0.0);
        color += material.specular * light.specular * RdotVpow * attenuation;
	}

if(lightingtype == 3 ) 
{  
		const float k = 0.8;
		vec3  n2 = normalize (Vert.normal);
		vec3  l2 = normalize (Vert.lightDir);
		vec3  v2 = normalize (Vert.viewDir);
		float d1 = pow(max(dot(n2, l2), 0.0), 1.0 + k);
		float d2 = pow(1.0 - dot(n2, v2), 1.0 - k);
		color = material.diffuse * d1 * d2;
}
	

	


    }
)";


void Init();
void Draw();
void Release();
//void InitTexture();

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
			else if (event.type == sf::Event::Resized) {
				glViewport(0, 0, event.size.width, event.size.height);
			}
			else if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case (sf::Keyboard::Right): xyz[0] += 0.1; break;
				case (sf::Keyboard::Up): xyz[1] += 0.1; break;
				case (sf::Keyboard::Left): xyz[0] -= 0.1; break;
				case (sf::Keyboard::Down): xyz[1] -= 0.1; break;
				case (sf::Keyboard::W): xyz[2] += 0.1; break;
				case (sf::Keyboard::S): xyz[2] -= 0.1; break;
				default: break;
				}
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
	//glGenBuffers(1, &textureVBO);
	Vertex3D * arr1 = new Vertex3D[edges.size()];
	for (int i = 0; i < edges.size(); i++) {
		arr1[i] = vert[edges[i].coord - 1];
	}
	Vertex3D * arr2 = new Vertex3D[edges.size()];
	for (int i = 0; i < edges.size(); i++) {
		arr2[i] = normals[edges[i].norm - 1];
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex3D) * edges.size(), arr1, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex3D) * edges.size(), arr2, GL_STATIC_DRAW);

	
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
	Attrib_vertex = glGetAttribLocation(Program, "position");
	if (Attrib_vertex == -1)
	{
		std::cout << "could not bind attrib " << "position" << std::endl;
		return;
	}


	vertColor = glGetAttribLocation(Program, "normal");
	if (vertColor == -1)
	{
		std::cout << "could not bind attrib " << "normal" << std::endl;
		return;
	}


	Unif_material_ambient = glGetUniformLocation(Program, "material.ambient");
	if (Unif_material_ambient == -1)
	{
		std::cout << "could not bind uniform material.ambient" << std::endl;
		return;
	}

	Unif_material_diffuse = glGetUniformLocation(Program, "material.diffuse");
	if (Unif_material_diffuse == -1)
	{
		std::cout << "could not bind uniform material.diffuse" << std::endl;
		return;
	}

	Unif_material_emission = glGetUniformLocation(Program, "material.emission");
	if (Unif_material_emission == -1)
	{
		std::cout << "could not bind uniform material.emission" << std::endl;
		return;
	}

	Unif_material_specular = glGetUniformLocation(Program, "material.specular");
	if (Unif_material_specular == -1)
	{
		std::cout << "could not bind uniform material.specular" << std::endl;
		return;
	}

	Unif_material_shininess = glGetUniformLocation(Program, "material.shininess");
	if (Unif_material_specular == -1)
	{
		std::cout << "could not bind uniform material.shininess" << std::endl;
		return;
	}

	Unif_light_position = glGetUniformLocation(Program, "light.position");
	if (Unif_light_position == -1)
	{
		std::cout << "could not bind uniform light.position" << std::endl;
		return;
	}

	Unif_light_ambient = glGetUniformLocation(Program, "light.ambient");
	if (Unif_light_ambient == -1)
	{
		std::cout << "could not bind uniform light.ambient" << std::endl;
		return;
	}

	Unif_light_diffuse = glGetUniformLocation(Program, "light.diffuse");
	if (Unif_light_diffuse == -1)
	{
		std::cout << "could not bind uniform light.diffuse" << std::endl;
		return;
	}

	Unif_light_specular = glGetUniformLocation(Program, "light.specular");
	if (Unif_light_specular == -1)
	{
		std::cout << "could not bind uniform light.specular" << std::endl;
		return;
	}

	Unif_light_attenuation = glGetUniformLocation(Program, "light.attenuation");
	if (Unif_light_attenuation == -1)
	{
		std::cout << "could not bind uniform light.attenuation" << std::endl;
		return;
	}

	Unif_trans = glGetUniformLocation(Program, "xyz");
	if (Unif_trans == -1)
	{
		std::cout << "could not bind uniform xyz" << std::endl;
		return;
	}

	checkOpenGLerror();
}

void Init() {
	InitShader();
	InitVBO();
	//InitTexture();
	// Включаем проверку глубины
	glEnable(GL_DEPTH_TEST);
}


void Draw() {
	// Устанавливаем шейдерную программу текущей
	glUseProgram(Program);
	//glUniform1f(unifAngle1, objectRotate1);
	//glUniform1f(unifAngle2, objectRotate2);

	glUniform3fv(Unif_transform_viewPosition, 1, transform.viewPosition);
	glUniform3fv(Unif_trans, 1, xyz);

	glUniform4fv(Unif_material_ambient, 1, material.ambient);
	glUniform4fv(Unif_material_diffuse, 1, material.diffuse);
	glUniform4fv(Unif_material_specular, 1, material.specular);
	glUniform4fv(Unif_material_emission, 1, material.emission);
	glUniform1f(Unif_material_shininess, material.shininess);

	glUniform3fv(Unif_light_position, 1, light.position);
	glUniform4fv(Unif_light_ambient, 1, light.ambient);
	glUniform4fv(Unif_light_diffuse, 1, light.diffuse);
	glUniform4fv(Unif_light_specular, 1, light.specular);
	glUniform3fv(Unif_light_attenuation, 1, light.attenuation);


	glEnableVertexAttribArray(Attrib_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glEnableVertexAttribArray(vertColor);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glVertexAttribPointer(vertColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Передаем данные на видеокарту(рисуем)
	glDrawArrays(GL_TRIANGLES, 0, edges.size());
	// Отключаем массив атрибутов
	glDisableVertexAttribArray(Attrib_vertex);
	glDisableVertexAttribArray(vertColor);
	//glDisableVertexAttribArray(attribTexture);
	// Отключаем шейдерную программу
	glUseProgram(0);
	checkOpenGLerror();



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
