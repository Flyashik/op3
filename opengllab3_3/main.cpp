#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>



// Переменные с индентификаторами ID
// ID шейдерной программы
GLuint Program;
// ID атрибута вершин
GLint Attrib_vertex;
// ID атрибута цвета
//GLint Attrib_color;

GLint Attrib_texture;
GLint Attrib_normal;
// ID VBO вершин
GLuint VBO;
// ID VBO цвета
GLuint VBO_color;

GLint Unif_rotate;
GLint Unif_texture;

GLint textureHandle;
sf::Texture textureData;

GLuint VAO;
GLuint IBO;

// Вершина
struct Vertex
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};


float rotate[3] = { 0.0f, 0.0f, 0.0f };
// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
    #version 330 core
    uniform vec3 rotate;

    //полученные атрибуты
    in vec3 vertex_pos;
    in vec2 texcoord;
    in vec3 normal;

    out vec2 tCoord;
    
    
    void main() {
        float x_angle = rotate[0];
        float y_angle = rotate[1];
        float z_angle = rotate[2];        

        vec3 vertex = vertex_pos 
        * mat3( cos(-y_angle), 0, -sin(-y_angle),
                0, 1, 0,
                sin(-y_angle), 0, cos(-y_angle) ) 
        * mat3( 1, 0, 0,
                0, cos(x_angle), -sin(x_angle),
                0, sin(x_angle), cos(x_angle) ) 
        * mat3( cos(z_angle), -sin(z_angle), 0,
                sin(z_angle), cos(z_angle), 0,
                0, 0, 1 );
        tCoord = texcoord;
        gl_Position = vec4(vertex, 1.0);
    }
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
    #version 330 core
    
    in vec2 tCoord;
    uniform sampler2D textureData;        
    out vec4 color;

    void main() {
        color = texture(textureData, tCoord);
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
            // обработка нажатий клавиш
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
    ReadFile("cube.obj");

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

    // Передаем вершины в буфер
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
    //АТРИБУТЫ
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
    //ЮНИФОРМЫ
    //-----------------------------------------------------------------
    //ставлю юниформу вращения
    Unif_rotate = glGetUniformLocation(Program, "rotate");
    if (Unif_rotate == -1)
    {
        std::cout << "could not bind uniform rotate " << std::endl;
        return;
    }
    //ставлю юниформу текстуры
    Unif_texture = glGetUniformLocation(Program, "textureData");
    if (Unif_texture == -1)
    {
        std::cout << "could not bind uniform textureData" << std::endl;
        return;
    }
    checkOpenGLerror();
}

void InitTexture()
{
    const char* filename = "123.jpg";
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
    // Устанавливаем шейдерную программу текущей
    glUseProgram(Program);
    // Включаем массивы атрибутов

    glEnable(GL_DEPTH_TEST);

    //текстура
    glActiveTexture(GL_TEXTURE0);
    sf::Texture::bind(&textureData);
    glUniform1i(Unif_texture, 0);
    //вращение
    glUniform3fv(Unif_rotate, 1, rotate);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, points.size());
    glBindVertexArray(0);

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
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Release() {
    ReleaseShader();
    ReleaseVBO();
}