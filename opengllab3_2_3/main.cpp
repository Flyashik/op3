
#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>



// ���������� � ����������������� ID
// ID ��������� ���������
GLuint Program;
// ID �������� ������
GLint Attrib_vertex;
// ID �������� �����
//GLint Attrib_color;

GLint Attrib_texture;
GLint Attrib_normal;
// ID VBO ������
GLuint VBO;
// ID VBO �����
GLuint VBO_color;

GLint Unif_rotate;
GLint Unif_texture;

GLint textureHandle;
sf::Texture textureData;

GLuint VAO;
//GLuint IBO;

// �������
struct Vertex
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

//��������� ��� ���������
//----------------------------------------------------
float viewPosition[3] = { 2.0, 2.0, 10.0 };

struct PointLight {
    float position[3] = { -3.0f, -3.0f, -3.0f };
    /*float ambient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float attenuation[3] = { 1.0, 0.0, 0.0 };*/
};

//struct Material {
//    float ambient[4] = { 0.24725f, 0.1995f, 0.0745f, 1.0f };
//    float diffuse[4] = { 0.75164f, 0.60648f, 0.22648f, 1.0f };
//    float specular[4] = { 0.628281f, 0.555802f, 0.366065f, 1.0f };
//    float emission[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
//    float shininess = 51.2f;
//};

PointLight light;
//Material material;

//��������
//transform
GLint trans_model;
GLint trans_viewProj;
GLint trans_normal;
GLint trans_viewPos;
//light
GLint light_pos;
GLint light_amb;
GLint light_diff;
GLint light_spec;
GLint light_atten;
//material
GLint mat_amb;
GLint mat_diff;
GLint mat_spec;
GLint mat_emiss;
GLint mat_shine;
//----------------------------------------------------

float rotate[3] = { 0.0f, 0.0f, 0.0f };
// �������� ��� ���������� �������
const char* VertexShaderSource = R"(
    #version 330 core

	uniform vec3 rotate;

    in vec3 vertex_pos;
	in vec2 texcoord;
	in vec3 normal;

	uniform vec3 viewPosition;

	uniform struct PointLight {
		vec3 position;
	} light;
    
	out Vertex {
		vec2 texcoord;
		vec3 normal;
		vec3 lightDir;
		vec3 viewDir;
		float distance;
	} Vert;

    void main() {
		float x_angle = rotate[0];
        float y_angle = rotate[1];
		float z_angle = rotate[2];
        
		vec3 vertex = vertex_pos * mat3(
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
		vec3 lightDir = light.position - vertex;
		gl_Position = vec4(vertex, 1.0);
		Vert.texcoord = texcoord;
		Vert.normal = normal * mat3(
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
		Vert.lightDir = vec3(lightDir);
		Vert.viewDir = viewPosition - vec3(vertex);
		Vert.distance = length(lightDir);
    }
)";

// �������� ��� ������������ �������
const char* FragShaderSource = R"(
    #version 330 core
    
    out vec4 color;

    uniform sampler2D textureData;

    in Vertex {
		vec2 texcoord;
		vec3 normal;
		vec3 lightDir;
		vec3 viewDir;
		float distance;
	} Vert;
	
    void main() {
		vec3 normal = normalize(Vert.normal);
		vec3 lightDir = normalize(Vert.lightDir);
		vec3 viewDir = normalize(Vert.viewDir);
	    
        const vec4 diffColor = vec4( 1.0, 1.0, 0.0, 1.0 );
        const float k = 0.8f;

        float d1 = pow(max(dot(normal, lightDir), 0.0), 1.0 + k);
        float d2 = pow(1.0 - dot(normal, viewDir), 1.0 - k);
        color = diffColor * d1 * d2 * texture(textureData, Vert.texcoord);

		
    }
)";


void Init();
void Draw();
void Release();

void rotate_change(float x, float y, float z)
{
    rotate[0] += x;
    rotate[1] += y;
    rotate[2] += z;
}

int main() {
    sf::Window window(sf::VideoMode(600, 600), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    window.setActive(true);

    // ������������� glew
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
            // ��������� ������� ������
            else if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case (sf::Keyboard::Up): rotate_change(0.05f, 0.0f, 0.0f); break;
                case (sf::Keyboard::Down): rotate_change(-0.05f, 0.0f, 0.0f); break;
                case (sf::Keyboard::Right): rotate_change(0.0f, 0.05f, 0.0f); break;
                case (sf::Keyboard::Left): rotate_change(0.0f, -0.05f, 0.0f); break;
                case (sf::Keyboard::PageUp): rotate_change(0.0f, 0.0f, 0.05f); break;
                case (sf::Keyboard::PageDown): rotate_change(0.0f, 0.0f, -0.05f); break;
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


// �������� ������ OpenGL, ���� ���� �� ����� � ������� ��� ������
void checkOpenGLerror() {
    GLenum errCode;
    // ���� ������ ����� �������� ���
    // https://www.khronos.org/opengl/wiki/OpenGL_Error
    if ((errCode = glGetError()) != GL_NO_ERROR)
        std::cout << "OpenGl error!: " << errCode << std::endl;
}

// ������� ������ ���� �������
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

struct Texture {
    GLfloat x;
    GLfloat y;
};

struct Normale
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct PolygPoint {
    Vertex vertex;
    Texture tex;
    Normale norm;
};


std::vector<Vertex> verticies;
std::vector<Normale> normales;
std::vector<Texture> textures;
//std::vector<int> vert_indexes;
//std::vector<int> tex_indexes;
//std::vector<int> norm_indexes;
//std::vector<int> indexes;
std::vector<PolygPoint> polypoints;

void ReadFile(std::string fname) {
    std::ifstream infile(fname);
    std::string dscrp;
    while (infile >> dscrp)
    {
        if (dscrp == "v")
        {
            float x, y, z;
            infile >> x >> y >> z;
            verticies.push_back({ x, y, z });
        }
        if (dscrp == "vt")
        {
            float x, y;
            infile >> x >> y;
            textures.push_back({ x, y });
        }
        if (dscrp == "vn")
        {
            float x, y, z;
            infile >> x >> y >> z;
            normales.push_back({ x, y, z });
        }
        if (dscrp == "f")
        {
            for (int i = 0; i < 3; i++)
            {
                char c;

                int vert_index;
                infile >> vert_index;
                infile >> c;
                //vert_indexes.push_back(vert_index - 1);

                int tex_index;
                infile >> tex_index;
                infile >> c;
                //tex_indexes.push_back(tex_index);

                int norm_index;
                infile >> norm_index;
                //norm_indexes.push_back(norm_index);

                polypoints.push_back({ verticies[vert_index - 1], textures[tex_index - 1], normales[norm_index - 1] });
            }
        }
    }

    //indexes = std::vector<int>(polypoints.size());

    //for (int i = 0; i < indexes.size(); i++)
    //    indexes[i] = i;
}


std::vector<GLfloat> points;

void InitVBO()
{
    ReadFile("file.obj");

    for (int i = 0; i < polypoints.size(); i++)
    {
        points.push_back(polypoints[i].vertex.x);
        points.push_back(polypoints[i].vertex.y);
        points.push_back(polypoints[i].vertex.z);
        points.push_back(polypoints[i].tex.x);
        points.push_back(polypoints[i].tex.y);
        points.push_back(polypoints[i].norm.x);
        points.push_back(polypoints[i].norm.y);
        points.push_back(polypoints[i].norm.z);
    }

    // �������� ������� � �����
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glEnableVertexAttribArray(Attrib_vertex);
    glEnableVertexAttribArray(Attrib_texture);
    glEnableVertexAttribArray(Attrib_normal);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(GLfloat), points.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glVertexAttribPointer(Attrib_texture, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(Attrib_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    checkOpenGLerror();
}



void InitShader() {
    // ������� ��������� ������
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    // �������� �������� ���
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    // ����������� ������
    glCompileShader(vShader);
    std::cout << "vertex shader \n";
    ShaderLog(vShader);

    // ������� ����������� ������
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    // �������� �������� ���
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
    // ����������� ������
    glCompileShader(fShader);
    std::cout << "fragment shader \n";
    ShaderLog(fShader);

    // ������� ��������� � ����������� ������� � ���
    Program = glCreateProgram();
    glAttachShader(Program, vShader);
    glAttachShader(Program, fShader);

    // ������� ��������� ���������
    glLinkProgram(Program);
    // ��������� ������ ������
    int link_ok;
    glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
    if (!link_ok)
    {
        std::cout << "error attach shaders \n";
        return;
    }
    //��������
    //-----------------------------------------------------------------
    Attrib_vertex = glGetAttribLocation(Program, "vertex_pos");
    if (Attrib_vertex == -1)
    {
        std::cout << "could not bind attrib vertex_pos" << std::endl;
        return;
    }
    Attrib_texture = glGetAttribLocation(Program, "texcoord");
    if (Attrib_texture == -1)
    {
        std::cout << "could not bind attrib texcoord" << std::endl;
        return;
    }
    Attrib_normal = glGetAttribLocation(Program, "normal");
    if (Attrib_normal == -1)
    {
        std::cout << "could not bind attrib normal" << std::endl;
        return;
    }
    //-----------------------------------------------------------------
    //��������
    //-----------------------------------------------------------------
    //������ �������� ��������
    Unif_rotate = glGetUniformLocation(Program, "rotate");
    if (Unif_rotate == -1)
    {
        std::cout << "could not bind uniform rotate " << std::endl;
        return;
    }
    //�������� �������� ��������
    Unif_texture = glGetUniformLocation(Program, "textureData");
    if (Unif_texture == -1)
    {
        std::cout << "could not bind uniform textureData" << std::endl;
        return;
    }
    //�������� �������� ��������������
    trans_viewPos = glGetUniformLocation(Program, "viewPosition");
    if (trans_viewPos == -1)
    {
        std::cout << "could not bind uniform viewPosition " << std::endl;
        return;
    }
    //�������� �������� ����� �����
    light_pos = glGetUniformLocation(Program, "light.position");
    if (light_pos == -1)
    {
        std::cout << "could not bind uniform light.position " << std::endl;
        return;
    }
    //light_amb = glGetUniformLocation(Program, "light.ambient");
    //if (light_amb == -1)
    //{
    //    std::cout << "could not bind uniform light.ambient " << std::endl;
    //    return;
    //}
    //light_diff = glGetUniformLocation(Program, "light.diffuse");
    //if (light_diff == -1)
    //{
    //    std::cout << "could not bind uniform light.diffuse " << std::endl;
    //    return;
    //}
    //light_spec = glGetUniformLocation(Program, "light.specular");
    //if (light_spec == -1)
    //{
    //    std::cout << "could not bind uniform light.specular " << std::endl;
    //    return;
    //}
    //light_atten = glGetUniformLocation(Program, "light.attenuation");
    //if (light_atten == -1)
    //{
    //    std::cout << "could not bind uniform light.attenuation " << std::endl;
    //    return;
    //}
    ////�������� �������� ���������
    //mat_amb = glGetUniformLocation(Program, "material.ambient");
    //if (mat_amb == -1)
    //{
    //    std::cout << "could not bind uniform material.ambient " << std::endl;
    //    return;
    //}
    //mat_diff = glGetUniformLocation(Program, "material.diffuse");
    //if (mat_diff == -1)
    //{
    //    std::cout << "could not bind uniform material.diffuse" << std::endl;
    //    return;
    //}
    //mat_spec = glGetUniformLocation(Program, "material.specular");
    //if (mat_spec == -1)
    //{
    //    std::cout << "could not bind uniform material.specular" << std::endl;
    //    return;
    //}
    //mat_emiss = glGetUniformLocation(Program, "material.emission");
    //if (mat_emiss == -1)
    //{
    //    std::cout << "could not bind uniform material.emission" << std::endl;
    //    return;
    //}
    //mat_shine = glGetUniformLocation(Program, "material.shininess");
    //if (mat_shine == -1)
    //{
    //    std::cout << "could not bind uniform material.shininess" << std::endl;
    //    return;
    //}
    //-----------------------------------------------------------------
    checkOpenGLerror();
}

void InitTexture()
{
    const char* filename = "gold-texture.jpg";
    if (!textureData.loadFromFile(filename))
    {
        return;
    }
    textureHandle = textureData.getNativeHandle();
}

void Init() {
    InitShader();
    InitVBO();
    InitTexture();
}

void Draw() {
    // ������������� ��������� ��������� �������
    glUseProgram(Program);
    // �������� ������� ���������

    glEnable(GL_DEPTH_TEST);

    //��������
    glActiveTexture(GL_TEXTURE0);
    sf::Texture::bind(&textureData);
    glUniform1i(Unif_texture, 0);
    //��������
    glUniform3fv(Unif_rotate, 1, rotate);

    //������� �������
    glUniform3fv(trans_viewPos, 1, viewPosition);

    //����
    glUniform3fv(light_pos, 1, light.position);
    //glUniform4fv(light_amb, 1, light.ambient);
    //glUniform4fv(light_diff, 1, light.diffuse);
    //glUniform4fv(light_spec, 1, light.specular);
    //glUniform3fv(light_atten, 1, light.attenuation);

    //��������
    /*glUniform4fv(mat_amb, 1, material.ambient);
    glUniform4fv(mat_diff, 1, material.diffuse);
    glUniform4fv(mat_spec, 1, material.specular);
    glUniform4fv(mat_emiss, 1, material.emission);
    glUniform1f(mat_shine, material.shininess);*/

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, points.size());
    glBindVertexArray(0);

    glUseProgram(0);
    checkOpenGLerror();
}


// ������������ ��������
void ReleaseShader() {
    // ��������� ����, �� ��������� �������� ���������
    glUseProgram(0);
    // ������� ��������� ���������
    glDeleteProgram(Program);
}

// ������������ ������
void ReleaseVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Release() {
    ReleaseShader();
    ReleaseVBO();
}