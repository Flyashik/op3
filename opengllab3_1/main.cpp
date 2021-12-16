// Вращающийся треугольник с текстурой (можно вращать стрелочками)

#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>


// В C и C++ есть оператор #, который позволяет превращать параметры макроса в строку
#define TO_STRING(x) #x


// Переменные с индентификаторами ID
// ID шейдерной программы
GLuint shaderProgram;
// ID атрибута вершин
GLint attribVertex;
// ID атрибута текстурных координат
GLint attribTexture1;
// ID юниформа текстуры
GLint unifTexture1;
GLint unifTexture2;
// ID юниформа угла поворота
GLint Unif_rotate;
// ID буфера ыершин
GLuint vertexVBO;
// ID буфера текстурных координат
GLuint textureVBO;
// ID текстуры
GLint textureHandle1;
// SFML текстура
sf::Texture textureData1;

GLint textureHandle2;
sf::Texture textureData2;

GLuint VBO_color;
GLint Attrib_color;

float objectRotation[3] = { 0.0f, 0.0f, 0.0f };


// Вершина
struct Vertex
{
    GLfloat x;
    GLfloat y;
};

struct Vertex3d
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};



// Шейдер это просто строка, и не кажно, каким образом она получена -
// можно загружать шейдеры из файла, можно объявлять прямо в программе,
// в том числе таким образом, при помощи специального макроса

// К сожалению, этот макрос не учитывает переводы строк, так что если они нужны,
// например, после дирректив препроцессора, нужно явно ставить символ '\n'

const char* VertexShaderSource = TO_STRING(
    #version 330 core\n


layout (location = 0) in vec3 vertCoord;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texureCoord1;


uniform vec3 rotate;
out vec2 tCoord;

out vec3 vert_color;

void main() {
    
    float x_angle = rotate[0];
    float y_angle = rotate[1];
    float z_angle = rotate[2];

    vec3 position = vertCoord
        * mat3(cos(-y_angle), 0, -sin(-y_angle),
            0, 1, 0,
            sin(-y_angle), 0, cos(-y_angle))
        * mat3(1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle))
        * mat3(cos(z_angle), -sin(z_angle), 0,
            sin(z_angle), cos(z_angle), 0,
            0, 0, 1);
    gl_Position = vec4(position, 1.0f);
    vert_color = color;
    tCoord = texureCoord1;
}
);

const char* FragShaderSource = TO_STRING(
    #version 330 core\n

   
    in vec2 tCoord;
    in vec3 vert_color;
    out vec4 color;
    uniform sampler2D textureData1;
    uniform sampler2D textureData2;
    
void main() {    
    color = mix(texture(textureData1, tCoord), texture(textureData2, tCoord), 0.5) * vec4(vert_color, 1.0f); 
}
);


void Init();
void Draw();
void Release();


int main() {
    sf::Window window(sf::VideoMode(600, 600), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    window.setActive(true);

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
                case (sf::Keyboard::Left): objectRotation[0] += 0.1; break;
                case (sf::Keyboard::Right): objectRotation[0] -= 0.1; break;
                case (sf::Keyboard::Up): objectRotation[1] += 0.1; break;
                case (sf::Keyboard::Down): objectRotation[1] -= 0.1; break;
                case (sf::Keyboard::PageUp): objectRotation[2] += 0.1; break;
                case (sf::Keyboard::PageDown): objectRotation[2] -= 0.1; break;
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
    glGenBuffers(1, &vertexVBO);
    glGenBuffers(1, &textureVBO);
    glGenBuffers(1, &VBO_color);

    // Объявляем вершины треугольника
    Vertex3d triangle[36] = {
        { -0.5, -0.5, 0.5 }, { -0.5, 0.5, 0.5 }, { 0.5, 0.5, 0.5 },
        { -0.5, -0.5, 0.5 }, { 0.5, 0.5, 0.5 }, { 0.5, -0.5, 0.5 },
        { -0.5, -0.5, 0.5 }, { -0.5, 0.5, 0.5 }, { -0.5, 0.5, -0.5 },
        { -0.5, -0.5, 0.5 }, { -0.5, 0.5, -0.5 }, { -0.5, -0.5, -0.5 },
        { 0.5, -0.5, 0.5 }, { 0.5, 0.5, 0.5 }, { 0.5, 0.5, -0.5 },
        { 0.5, -0.5, 0.5 }, { 0.5, 0.5, -0.5 }, { 0.5, -0.5, -0.5 },
        { -0.5, -0.5, 0.5 }, { -0.5, -0.5, -0.5 }, { 0.5, -0.5, -0.5 },
        { -0.5, -0.5, 0.5 }, { 0.5, -0.5, -0.5 }, { 0.5, -0.5, 0.5 },
        { -0.5, 0.5, 0.5 }, { -0.5, 0.5, -0.5 }, { 0.5, 0.5, -0.5 },
        { -0.5, 0.5, 0.5 }, { 0.5, 0.5, -0.5 }, { 0.5, 0.5, 0.5 },
        { -0.5, -0.5, -0.5 }, { -0.5, 0.5, -0.5 }, { 0.5, 0.5, -0.5 },
        { -0.5, -0.5, -0.5 }, { 0.5, 0.5, -0.5 }, { 0.5, -0.5, -0.5 },
    };

    // Объявляем текстурные координаты
    Vertex3d texture[36] = {
        { 0.0, 0.0 }, { 0.0, 1.0 }, { 1.0, 1.0 },
        { 0.0, 0.0 }, { 1.0, 1.0 }, { 1.0, 0.0 },
        { 0.0, 0.0 }, { 0.0, 1.0 }, { 1.0, 1.0 },
        { 0.0, 0.0 }, { 1.0, 1.0 }, { 1.0, 0.0 },
        { 0.0, 0.0 }, { 0.0, 1.0 }, { 1.0, 1.0 },
        { 0.0, 0.0 }, { 1.0, 1.0 }, { 1.0, 0.0 },
        { 0.0, 0.0 }, { 0.0, 1.0 }, { 1.0, 1.0 },
        { 0.0, 0.0 }, { 1.0, 1.0 }, { 1.0, 0.0 },
        { 0.0, 0.0 }, { 0.0, 1.0 }, { 1.0, 1.0 },
        { 0.0, 0.0 }, { 1.0, 1.0 }, { 1.0, 0.0 },
        { 0.0, 0.0 }, { 0.0, 1.0 }, { 1.0, 1.0 },
        { 0.0, 0.0 }, { 1.0, 1.0 }, { 1.0, 0.0 },
    };
    float colors[36][3] = {
        { 1.0, 1.0, 1.0 }, { 1.0, 1.0, 0.0 }, { 0.0, 1.0, 1.0 },
        { 1.0, 1.0, 1.0 }, { 0.0, 1.0, 1.0 }, { 1.0, 0.0, 1.0 },
        { 1.0, 1.0, 1.0 }, { 1.0, 1.0, 0.0 }, { 1.0, 0.0, 0.0 },
        { 1.0, 1.0, 1.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 },
        { 1.0, 0.0, 1.0 }, { 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 },
        { 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 }, { 0.5, 0.5, 0.5 },
        { 1.0, 1.0, 1.0 }, { 0.0, 1.0, 0.0 }, { 0.5, 0.5, 0.5 },
        { 1.0, 1.0, 1.0 }, { 0.5, 0.5, 0.5 }, { 1.0, 0.0, 1.0 },
        { 1.0, 1.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 },
        { 1.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 1.0, 1.0 },
        { 0.0, 1.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 },
        { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.5, 0.5, 0.5 },
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture), texture, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    checkOpenGLerror();
}


void InitShader() {
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    glCompileShader(vShader);
    std::cout << "vertex shader \n";
    ShaderLog(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
    glCompileShader(fShader);
    std::cout << "fragment shader \n";
    ShaderLog(fShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vShader);
    glAttachShader(shaderProgram, fShader);

    glLinkProgram(shaderProgram);
    int link_status;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &link_status);
    if (!link_status)
    {
        std::cout << "error attach shaders \n";
        return;
    }

    attribVertex = glGetAttribLocation(shaderProgram, "vertCoord");
    if (attribVertex == -1)
    {
        std::cout << "could not bind attrib vertCoord" << std::endl;
        return;
    }

    attribTexture1 = glGetAttribLocation(shaderProgram, "texureCoord1");
    if (attribTexture1 == -1)
    {
        std::cout << "could not bind attrib texureCoord1" << std::endl;
        return;
    }


    unifTexture1 = glGetUniformLocation(shaderProgram, "textureData1");
    if (unifTexture1 == -1)
    {
        std::cout << "could not bind uniform textureData1" << std::endl;
        return;
    }

    unifTexture2 = glGetUniformLocation(shaderProgram, "textureData2");
    if (unifTexture2 == -1)
    {
        std::cout << "could not bind uniform textureData2" << std::endl;
        return;
    }

    Unif_rotate = glGetUniformLocation(shaderProgram, "rotate");
    if (Unif_rotate == -1)
    {
        std::cout << "could not bind uniform rotate" << std::endl;
        return;
    }

    Attrib_color = glGetAttribLocation(shaderProgram, "color");
    if (Attrib_color == -1)
    {
        std::cout << "could not bind attrib color" << std::endl;
        return;
    }
    checkOpenGLerror();
}

void InitTexture()
{
    const char* filename = "image2.png";
    // Загружаем текстуру из файла
    if (!textureData1.loadFromFile(filename))
    {
        // Не вышло загрузить картинку
        return;
    }
    // Теперь получаем openGL дескриптор текстуры
    textureHandle1 = textureData1.getNativeHandle();

    filename = "image.jpg";
    if (!textureData2.loadFromFile(filename))
    {
        // Не вышло загрузить картинку
        return;
    }
    // Теперь получаем openGL дескриптор текстуры
    textureHandle2 = textureData2.getNativeHandle();
}

void Init() {
    InitShader();
    InitVBO();
    InitTexture();
}


void Draw() {
    // Устанавливаем шейдерную программу текущей
    glUseProgram(shaderProgram);
    // Передаем юниформ в шейдер
    glUniform3fv(Unif_rotate, 1, objectRotation);
    glEnable(GL_DEPTH_TEST);

    // Активируем текстурный блок 0, делать этого не обязательно, по умолчанию
    // и так активирован GL_TEXTURE0, это нужно для использования нескольких текстур
    glActiveTexture(GL_TEXTURE0);
    // Обёртка SFML на opengl функцией glBindTexture
    sf::Texture::bind(&textureData1);
    // В uniform кладётся текстурный индекс текстурного блока (для GL_TEXTURE0 - 0, для GL_TEXTURE1 - 1 и тд)
    glUniform1i(unifTexture1, 0);

    glActiveTexture(GL_TEXTURE1);
    // Обёртка SFML на opengl функцией glBindTexture
    sf::Texture::bind(&textureData2);
    // В uniform кладётся текстурный индекс текстурного блока (для GL_TEXTURE0 - 0, для GL_TEXTURE1 - 1 и тд)
    glUniform1i(unifTexture2, 1);
    
    // Подключаем VBO
    glEnableVertexAttribArray(attribVertex);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attribTexture1);
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glVertexAttribPointer(attribTexture1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    

    glEnableVertexAttribArray(Attrib_color);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
    glVertexAttribPointer(Attrib_color, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Передаем данные на видеокарту(рисуем)
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Отключаем массив атрибутов
    glDisableVertexAttribArray(attribVertex);
    glDisableVertexAttribArray(Attrib_color);
    // Отключаем шейдерную программу
    glUseProgram(0);
    checkOpenGLerror();
}


void ReleaseShader() {
    glUseProgram(0);
    glDeleteProgram(shaderProgram);
}

void ReleaseVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vertexVBO);
    glDeleteBuffers(1, &VBO_color);
}

void Release() {
    ReleaseShader();
    ReleaseVBO();
}