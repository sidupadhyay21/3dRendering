#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <time.h>
using namespace std;

GLFWwindow* window;
GLuint vertexbuffer;
GLuint colorbuffer;
GLuint normalbuffer;
GLuint MatrixID;
GLuint VertexArrayIDcube;
GLuint ModelID;
GLuint ProjectionID;
GLuint ViewID;
GLuint LightID;

glm::mat4 Projection;

glm::mat4 ViewCamera;

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

class obj {
public:
	GLuint bufferID_v;
	GLuint bufferID_vt;
	GLuint bufferID_vn;
	GLuint bufferID_c;
	GLuint vao_id;
	float* vbuffer;
	float* vnbuffer;
	float* vtbuffer;
	float* cbuffer;
	int vertexCount;
	obj(string name) {
		ifstream filein;
		filein.open(name);
		string line;
		vector <float> vertices;
		vector <float> vt;
		vector <float> vn;
		vector <int> f;
		while (getline(filein, line)) {
			if (line.length() > 0) {
				istringstream iss(line);
				if (line.substr(0, 2) == "v ") {
					float vertexx, vertexy, vertexz;
					string temp;
					iss >> temp >> vertexx >> vertexy >> vertexz;
					//cout << temp << vertexx << vertexy << vertexz << endl;
					vertices.push_back(vertexx);
					vertices.push_back(vertexy);
					vertices.push_back(vertexz);
				}
				if (line.substr(0, 2) == "vt") {
					float uvx, uvy;
					string temp;
					iss >> temp >> uvx >> uvy;
					//cout << temp << uvx << uvy << endl;
					vt.push_back(uvx);
					vt.push_back(uvy);
				}
				if (line.substr(0, 2) == "vn") {
					float normalx, normaly, normalz;
					string temp;
					iss >> temp >> normalx >> normaly >> normalz;
					//cout << temp << normalx << normaly << normalz << endl;
					vn.push_back(normalx);
					vn.push_back(normaly);
					vn.push_back(normalz);
				}
				if (line.substr(0, 2) == "f ") {
					vector <string> tokens;
					string temp;
					stringstream check1(line);
					while (getline(check1, temp, ' ')) {
						tokens.push_back(temp);
					}
					for (int i = 1; i < tokens.size(); i++) {
						stringstream check2(tokens[i]);
						while (getline(check2, temp, '/')) {
							f.push_back(stoi(temp));
							//cout << "f " << temp << endl;
						}

					}
				}
			}
		}
		//set size of array pointers
		vertexCount = f.size()/3;
		vbuffer = new float[vertexCount * 3];
		vtbuffer = new float[vertexCount * 2];
		vnbuffer = new float[vertexCount * 3];
		cbuffer = new float[vertexCount * 3];
		for (int i = 0; i < vertexCount; i += 1) {
			int v_index = f[i*3] - 1;
			int vt_index = f[i*3 + 1] - 1;
			int vn_index = f[i*3 + 2] - 1;
			vbuffer[i * 3] = vertices[v_index * 3];
			vbuffer[i * 3 + 1] = vertices[v_index * 3 + 1];
			vbuffer[i * 3 + 2] = vertices[v_index * 3 + 2];
			vtbuffer[i * 2] = vt[vt_index * 2];
			vtbuffer[i * 2 + 1] = vt[vt_index * 2 + 1];
			vnbuffer[i * 3] = vn[vn_index * 3];
			vnbuffer[i * 3 + 1] = vn[vn_index * 3 + 1];
			vnbuffer[i * 3 + 2] = vn[vn_index * 3 + 2];
			cbuffer[i * 3] = 0;
			cbuffer[i * 3 + 1] = 1;
			cbuffer[i * 3 + 2] = 0;
		}		

		/*glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * faceCount, vtbuffer, GL_STATIC_DRAW);
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * faceCount, vnbuffer, GL_STATIC_DRAW);*/
	}

	~obj() {
		delete[] vbuffer;
		delete[] vnbuffer;
		delete[] vtbuffer;
		delete[] cbuffer;
	}

	void drawObj() {
		/*
		glm::vec3 rotateV = glm::vec3(0,0,0);
		glm::mat4 Model = glm::mat4(1.0f);
		Model = glm::translate(Model, glm::vec3(xtranslate, ytranslate, ztranslate));
		Model = glm::rotate(Model, rotateV.x, glm::vec3(xrotate, 0, 0));
		Model = glm::rotate(Model, rotateV.y, glm::vec3(0, yrotate, 0));
		Model = glm::rotate(Model, rotateV.z, glm::vec3(0, 0, zrotate));
		//Model = glm::scale(Model, glm::vec3(0.1f));
		//glm::mat4 MVP = Projection * ViewCamera * Model;
		glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Model[0][0]);
		glUniformMatrix4fv(ViewID, 1, GL_FALSE, &ViewCamera[0][0]);
		glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &Projection[0][0]);
		glUniform3f(LightID, 100.0f, 100.0f, -100.0f);
		*/
		// 1st attribute buffer : vertices
		glGenBuffers(1, &bufferID_v);
		glBindBuffer(GL_ARRAY_BUFFER, bufferID_v);
		glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vbuffer, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		// glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : colors
		glGenBuffers(1, &bufferID_c);
		glBindBuffer(GL_ARRAY_BUFFER, bufferID_c);
		glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), cbuffer, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(
			1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 3rd attribute buffer : normals
		glGenBuffers(1, &bufferID_vn);
		glBindBuffer(GL_ARRAY_BUFFER, bufferID_vn);
		glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vnbuffer, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		// glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertexCount); // 12*3 indices starting at 0 -> 12 triangles
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
};

float cameraX = 0;
float cameraY = 1;
float cameraZ = 0;
int rotateX = 0, rotateY = 0, rotateZ = 0;
int screenWidth, screenHeight;
obj *testobj;
obj *cube;

double d2r(double d) {
	return (d / 180.0 * 3.141592);
}

const float g_normal_buffer_data[] = {
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
};

static const GLfloat g_vertex_buffer_data[] = {
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
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

// One color for each vertex. They were generated randomly.
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

float cubeFaceColors[72] = {
	1,0,0,  1,0,0,  1,0,0,  1,0,0,      // face #1 is red
	0,1,0,  0,1,0,  0,1,0,  0,1,0,      // face #2 is green
	0,0,1,  0,0,1,  0,0,1,  0,0,1,      // face #3 is blue
	1,1,0,  1,1,0,  1,1,0,  1,1,0,      // face #4 is yellow
	0,1,1,  0,1,1,  0,1,1,  0,1,1,      // face #5 is cyan
	1,0,1,  1,0,1,  1,0,1,  1,0,1		// face #6 is red
};
const GLfloat vertex_array[] = {
	1, 1, 1,
	-1, 1, 1,
	-1,-1, 1,
	1,-1, 1,   // v0,v1,v2,v3 (front)
	1, 1, 1,
	1,-1, 1,
	1,-1,-1,
	1, 1,-1,   // v0,v3,v4,v5 (right)
	1, 1, 1,
	1, 1,-1,
	-1, 1,-1,
	-1, 1, 1,   // v0,v5,v6,v1 (top)
	-1, 1, 1,
	-1, 1,-1,
	-1,-1,-1,
	-1,-1, 1,   // v1,v6,v7,v2 (left)
	-1,-1,-1,
	1,-1,-1,
	1,-1, 1,
	-1,-1, 1,   // v7,v4,v3,v2 (bottom)
	1,-1,-1,
	-1,-1,-1,
	-1, 1,-1,
	1, 1,-1    // v4,v7,v6,v5 (back)
};
const GLubyte index_array[] = {
0, 1, 2,
2, 3, 0, // front
4, 5, 6,
6, 7, 4, // right
8,9,10,
10,11,8, // top
12,13,14,
14,15,12, // left
16,17,18,
18,19,16, // bottom
20,21,22,
22,23,20  // back
};
const GLubyte line_array[] = {
0, 1, 1, 2, 2, 0,
2, 3, 3, 0, 0, 2,// front
4, 5, 5, 6, 6, 4,
6, 7, 7, 4, 4, 6, // right
8,9,9,10,10,8,
10,11,11,8,8,10, // top
12,13,13,14,14,12,
14,15,15,12,12,14, // left
16,17,17,18,18,16,
18,19,19,16,16,18, // bottom
20,21,21,22,22,20,
22,23,23,20,20,22  // back
};

void drawCube() {
	
	// 1st attribute buffer : vertices
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	// glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : colors
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	//glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : normals
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_normal_buffer_data), g_normal_buffer_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	//glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	
	// Draw the triangle !
	//glBindVertexArray(VertexArrayIDcube);
	glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

class GameObject {
public:
	double rotateX;
	double rotateY;
	double rotateZ;
	double tX;
	double tY;
	double tZ;
	obj *mesh;
	GameObject(double startX, double startY, double startZ) {
		this->rotateX = 0;
		this->rotateY = 0;
		this->rotateZ = 0;
		this->tX = startX;
		this->tY = startY;
		this->tZ = startZ;
	}
	GameObject(double startX, double startY, double startZ, obj *mesh) {
		this->rotateX = 0;
		this->rotateY = 0;
		this->rotateZ = 0;
		this->tX = startX;
		this->tY = startY;
		this->tZ = startZ;
		this->mesh = mesh;
	}
	void render() {
		glm::vec3 rotateV = glm::vec3(d2r(this->rotateX), d2r(this->rotateY), d2r(this->rotateZ));
		glm::mat4 Model = glm::mat4(1.0f);
		Model = glm::translate(Model, glm::vec3(this->tX, this->tY, this->tZ));
		Model = glm::rotate(Model, rotateV.x, glm::vec3(-1, 0, 0));
		Model = glm::rotate(Model, rotateV.y, glm::vec3(0, -1, 0));
		Model = glm::rotate(Model, rotateV.z, glm::vec3(0, 0, -1));
		//glm::mat4 MVP = Projection * ViewCamera * Model;
		glUniformMatrix4fv(ModelID, 1, GL_FALSE, &Model[0][0]);
		glUniformMatrix4fv(ViewID, 1, GL_FALSE, &ViewCamera[0][0]);
		glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &Projection[0][0]);
		glUniform3f(LightID, 100.0f, 100.0f, -100.0f);
		if (this->mesh == nullptr){
			drawCube();
		}
		else {
			this->mesh->drawObj();
		}
		
	}
	bool overlap(GameObject b) {
		
	}
};

/*
void resize(int w, int h){
	screenWidth = w;
	screenHeight = h;
	glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (float)(screenWidth) / screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
*/
/*
GameObject cube1(0, 1, 20);
GameObject cube2(0, 3, 20);
GameObject cube3(0, 5, 20);
GameObject cube4(2, 3, 20);
GameObject cube5(-4, 3, 20);
GameObject cube6(-4, 1, 20);
GameObject cube7(-4, 5, 20);
GameObject cube8(4, 1, 20);
GameObject cube9(4, 3, 20);
GameObject cube10(4, 5, 20);
*/
GameObject *cube1;
GameObject *cube2;
GameObject *cube3;
GameObject *cube4;
GameObject *cube5;
GameObject *cube6;
GameObject *cube7;
GameObject *cube8;
GameObject *cube9;
GameObject *cube10;
GameObject *player;
GameObject *obj1;
GameObject* obj2;
GameObject* obj3;

void drawletters() {
	cube1->render();
	cube2->render();
	cube3->render();
	cube4->render();
	cube5->render();
	cube6->render();
	cube7->render();
	cube8->render();
	cube9->render();
	cube10->render();
	obj1->render();
	obj2->render();
	obj3->render();
}

void updateCamera() {
	
	float r = d2r(player->rotateY);
	cameraZ = player->tZ - 15*(cos(r));
	cameraX = player->tX + 15*(sin(r));
	cameraY = player->tY + 5;
			
	Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	ViewCamera = glm::lookAt(
		glm::vec3(cameraX, cameraY, cameraZ), // Camera location
		glm::vec3(player->tX, player->tY, player->tZ), // point the camera is focused on
		glm::vec3(0, 1, 0)  // "up" direction camera
	);
}

void drawFrame(){
	updateCamera();
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective(60.0f, (float)(screenWidth) / screenHeight, 0.1f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBegin(GL_QUADS);
	glColor3f(0.9, 1.0, 1.0);
	glVertex3f(-100, -2, 100);
	glVertex3f(-100, -2, -100);
	glVertex3f(100, -2, -100);
	glVertex3f(100, -2, 100);
	glEnd();
	drawletters();
	player->render();
	glfwSwapBuffers(window);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	double r = d2r(-player->rotateY);
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		player->tZ -= cos(r);
		player->tX -= sin(r);
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		player->tZ += cos(r);
		player->tX += sin(r);
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		player->tZ += sin(r);
		player->tX -= cos(r);
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		player->tZ -= sin(r);
		player->tX += cos(r);
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		player->rotateY += 15;
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		player->rotateY -= 15;
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		player->tY += 2;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		player->tY -= 2;
}

int main() {
	if (!glfwInit()) {
		fprintf(stderr, "error");
		return -1;
	}
	window = glfwCreateWindow(1024, 768, "Test", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "error");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "error");
		glfwTerminate();
		return -1;
	}
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glClearColor(0.3f, 0.5f, 0.8f, 0.0f);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	// connect with the uniform variable in the shader
	ModelID = glGetUniformLocation(programID, "model");
	ViewID = glGetUniformLocation(programID, "view");
	ProjectionID = glGetUniformLocation(programID, "projection");
	LightID = glGetUniformLocation(programID, "lightPos");

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	glDisable(GL_CULL_FACE);

	glfwSetKeyCallback(window, key_callback);
	srand(time(NULL));

	testobj = new obj("tree2.obj");
	cube = new obj("basic_cube3");
	player = new GameObject(0, 1, 0, cube);
	obj1 = new GameObject(rand() % 20 * -1, 1, rand() % 20 * -1, testobj);
	obj2 = new GameObject(rand() % 20 * -1, 1, rand() % 20 * -1, testobj);
	obj3 = new GameObject(rand() % 20 * -1, 1, rand() % 20 * -1, testobj);
	cube1 = new GameObject(0, 1, 20, cube);
	cube2 = new GameObject(0, 3, 20, cube);
	cube3 = new GameObject(0, 5, 20, cube);
	cube4 = new GameObject(2, 3, 20, cube);
	cube5 = new GameObject(-4, 3, 20, cube);
	cube6 = new GameObject(-4, 1, 20, cube);
	cube7 = new GameObject(-4, 5, 20, cube);
	cube8 = new GameObject(4, 1, 20, cube);
	cube9 = new GameObject(4, 3, 20, cube);
	cube10 = new GameObject(4, 5, 20, cube);

	do {
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Use our shader
		glUseProgram(programID);

		// Swap buffers
		drawFrame();
		glfwPollEvents();
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);

	delete testobj;
	glfwTerminate();
	return 0;
}