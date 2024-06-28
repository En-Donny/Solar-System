#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gl/GL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shaders.h"
#include "Planets.h"
#include "stb_image.h"
#include <stdlib.h> // нужен для вызова функций rand(), srand()
#include <time.h> // нужен для вызова функции time()
#include <iostream>
#include <vector>
#include <math.h>
#include <map>
#include <string>
#include <ft2build.h>
#include <windows.h>
#include FT_FREETYPE_H 
#include "Camera.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mousePress_callback(GLFWwindow* window, int button, int action, int mods);
void RenderText(Shader& shader, std::string text, float x, float y, float z, float scale, glm::vec3 color);

unsigned int loadCubemap(vector<string> faces);

// Константы
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Содержит всю информацию о состоянии символа, загруженному с помощью библиотеки FreeType
struct Character {
    unsigned int TextureID; // ID текстуры глифа
    glm::ivec2   Size;      // размер глифа
    glm::ivec2   Bearing;   // смещение от линии шрифта до верхнего/левого угла глифа
    unsigned int Advance;   // смещение до следующего глифа
};

//коллекция с со всеми возможными символами 
std::map<GLchar, Character> Characters;
unsigned int textVAO, textVBO;

// Камера
Camera camera(glm::vec3(0.0f, 200.0f, 700.0f));

// Тайминги
float currentFrame;
float deltaTime = 0.0f;	// время между текущим кадром и последним кадром
float lastFrame = 0.0f;

float deltaTimeZalip = 0.0f;	// переменные для определения залипания клавиш
float lastFrameZalip = 0.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouseMoving = true;

const vector<float> sphereVertices = initsphereVertices(Y_SEGMENTS, X_SEGMENTS);
const vector<int> sphereIndices = initsphereIndices(Y_SEGMENTS, X_SEGMENTS);

const vector<float> meteorVertices = initsphereVertices(Y_SEGMENTS_METEOR, X_SEGMENTS_METEOR);
const vector<int> meteorIndices = initsphereIndices(Y_SEGMENTS_METEOR, X_SEGMENTS_METEOR);
List <Planet> SolarSystem;
List<glm::vec3> PlanetCoordinates;

int StopPlanetsMoving = 1;//если переносимся к планете - тормозим движение планет вокруг Солнца

int ChoosenPlanet = 999;//идентификатор выбранной с помощью кнопок планеты

bool HideTrajectories = false;//флаг скрытия траекторий движения планет
bool HidePlanetNames = false;//флаг скрытия названий планет

double SpeedMultiplier = 1;//множитель скорости движения планет

int main()
{
    srand(time(NULL));
    // glfw: инициализация и конфигурирование    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw: создание окна
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mousePress_callback);
    // Сообщаем GLFW, чтобы он захватил наш курсор
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: загрузка всех указателей на OpenGL-функции
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);

    // Компилирование нашей шейдерной программы
    static Shader PlanetShader("Shaders/vertexShader.txt", "Shaders/fragmentShader.txt");

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //Generate and bind the VAO and VBO of the sphere
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Bind the vertex data to the current default buffer
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

    GLuint EBO;//EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);

    //Устанавливаем параметры, передаваемые в вершинный шейдер
    //передаем в качестве первого параметра 3 координаты вершины
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    ///затем - 2 координаты текстуры этой вершины
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    PlanetShader.use(); // не забыть активировать шейдер перед настройкой uniform-переменных!  

    static Shader meteorShader("Shaders/meteorVertexShader.txt", "Shaders/fragmentShader.txt");

    unsigned int VBO1, VAO1;

    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);
    //Generate and bind the VAO and VBO of the sphere
    glBindVertexArray(VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    // Bind the vertex data to the current default buffer
    glBufferData(GL_ARRAY_BUFFER, meteorVertices.size() * sizeof(float), &meteorVertices[0], GL_STATIC_DRAW);

    GLuint EBO1;//EBO

    glGenBuffers(1, &EBO1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meteorIndices.size() * sizeof(int), &meteorIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //
    ////// Указываем OpenGL какой сэмплер к какому текстурному блоку принадлежит (это нужно сделать единожды) 
    meteorShader.use(); // не забудьте активировать шейдер перед настройкой uniform-переменных!  

    Shader skyboxShader("Shaders/skyVertexShader.txt", "Shaders/skyFragmentShader.txt");

    float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
    };

    // VAO скайбокса
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    vector<string> faces//на заметку: скайбокс нормально рендерится только с квадратными изображениями
    {
        "textures/Planets/sky1.jpg",
        "textures/Planets/sky1.jpg",
        "textures/Planets/sky1.jpg",
        "textures/Planets/sky1.jpg",
        "textures/Planets/sky1.jpg",
        "textures/Planets/sky1.jpg"
    };

    unsigned int cubemapTexture = loadCubemap(faces);
    skyboxShader.use();

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // установка параметров наложения текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // установка метода наложения текстуры GL_REPEAT (стандартный метод наложения)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Установка параметров фильтрации текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Загрузка изображения, создание текстуры и генерирование мипмап-уровней
    int width, height, nrChannels;

    unsigned char* data = stbi_load("textures/Planets/saturn_ring1.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
    const int segmentsCirc = 360;
    const int torusCircNum = 360;
    const int minorRadius = 30;
    const int majorRadius = 50;

    vector<float> ringVertices;
    for (int r = minorRadius; r <= majorRadius; ++r) {
        for (int i = 0; i <= segmentsCirc; ++i) {
            float xSegment = (float)i / (float)segmentsCirc;
            float ringSegment;
            ringSegment = (float)(majorRadius - r) / (float)majorRadius;
            float x = r * cos(2.0f * PI * xSegment);
            float z = r * sin(2.0f * PI * xSegment);
            ringVertices.push_back(x);
            ringVertices.push_back(0);
            ringVertices.push_back(z);
            ringVertices.push_back(ringSegment);
            ringVertices.push_back(xSegment);
        }
    }

    vector<int> ringIndeces;
    for (int r = 0; r < (majorRadius - minorRadius); ++r) {
        for (int i = 0; i < segmentsCirc; ++i) {
            ringIndeces.push_back(r * (segmentsCirc + 1) + i);
            ringIndeces.push_back((r + 1) * (segmentsCirc + 1) + i);
            ringIndeces.push_back((r + 1) * (segmentsCirc + 1) + i + 1);
            ringIndeces.push_back(r * (segmentsCirc + 1) + i);
            ringIndeces.push_back((r + 1) * (segmentsCirc + 1) + i + 1);
            ringIndeces.push_back(r * (segmentsCirc + 1) + i + 1);
        }
    }

    static Shader ringShader("Shaders/vertexShader.txt", "Shaders/ringFragmentShader.txt");

    unsigned int ringVBO, ringVAO;
    glGenVertexArrays(1, &ringVAO);
    glGenBuffers(1, &ringVBO);
    //Generate and bind the VAO and VBO of the sphere
    glBindVertexArray(ringVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ringVBO);
    // Bind the vertex data to the current default buffer
    glBufferData(GL_ARRAY_BUFFER, ringVertices.size() * sizeof(float), &ringVertices[0], GL_STATIC_DRAW);

    GLuint ringEBO;//EBO
    glGenBuffers(1, &ringEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ringEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ringIndeces.size() * sizeof(int), &ringIndeces[0], GL_STATIC_DRAW);

    //Set the vertex attribute pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    ringShader.use(); // не забудьте активировать шейдер перед настройкой uniform-переменных!  

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    vector<float> torusVertices;
    for (int i = 0; i <= torusCircNum; ++i) {
        for (int j = 0; j <= segmentsCirc; ++j) {
            float xSegment = (float)j / (float)segmentsCirc;
            float ringSegment;
            ringSegment = (float)i / (float)torusCircNum;
            float x = (122 + 0.08 * cos(2.0f * PI * ringSegment)) * cos(2.0f * PI * xSegment);
            float z = (122 + 0.08 * cos(2.0f * PI * ringSegment)) * sin(2.0f * PI * xSegment);
            float y = 0.08 * sin(2.0f * PI * ringSegment);
            torusVertices.push_back(x);
            torusVertices.push_back(y);
            torusVertices.push_back(z);
            torusVertices.push_back(xSegment);
            torusVertices.push_back(ringSegment);
        }
    }

    vector<int> torusIndeces;
    for (int r = 0; r < torusCircNum; ++r) {
        for (int i = 0; i < segmentsCirc; ++i) {
            torusIndeces.push_back(r * (segmentsCirc + 1) + i);
            torusIndeces.push_back((r + 1) * (segmentsCirc + 1) + i);
            torusIndeces.push_back((r + 1) * (segmentsCirc + 1) + i + 1);
            torusIndeces.push_back(r * (segmentsCirc + 1) + i);
            torusIndeces.push_back((r + 1) * (segmentsCirc + 1) + i + 1);
            torusIndeces.push_back(r * (segmentsCirc + 1) + i + 1);
        }
    }

    List<float> CoeffsTorusRadius;
    CoeffsTorusRadius.pushElement(1.0f);
    CoeffsTorusRadius.pushElement(0.5f);
    CoeffsTorusRadius.pushElement(0.7f);
    CoeffsTorusRadius.pushElement(1.3f);
    CoeffsTorusRadius.pushElement(4.0f);
    CoeffsTorusRadius.pushElement(7.0f);
    CoeffsTorusRadius.pushElement(9.0f);
    CoeffsTorusRadius.pushElement(11.0f);

    static Shader torusShader("Shaders/vertexShader.txt", "Shaders/trajectoriesFragmentShader.txt");

    unsigned int torusVBO, torusVAO;
    glGenVertexArrays(1, &torusVAO);
    glGenBuffers(1, &torusVBO);
    //Generate and bind the VAO and VBO of the sphere
    glBindVertexArray(torusVAO);
    glBindBuffer(GL_ARRAY_BUFFER, torusVBO);
    // Bind the vertex data to the current default buffer
    glBufferData(GL_ARRAY_BUFFER, torusVertices.size() * sizeof(float), &torusVertices[0], GL_STATIC_DRAW);

    GLuint torusEBO;//EBO
    glGenBuffers(1, &torusEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torusEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, torusIndeces.size() * sizeof(int), &torusIndeces[0], GL_STATIC_DRAW);

    //Set the vertex attribute pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    torusShader.use(); // не забудьте активировать шейдер перед настройкой uniform-переменных!  

    Planet Sun(48, 0, "Sun", "textures/Planets/sun1.jpg", glm::vec3(0.0f, 0.0f, 0.0f), 1, 0.005f, 0.0f);

    float ShablonRadius = ((Sun.getRadius() * 12.0) / 109.0) * 0.375;//считаем радиус Земли за шаблон для масштабирования

    //Земля с Луной
    Planet Earth(((Sun.getRadius() * 12.0) / 109.0) * 0.375, 122, "Earth", "textures/Planets/earth.jpg", glm::vec3(5.0f, 0.0f, -5.0f), 1.0f, 0.4f, 23.4f);
    Sputnik Moon(0.17 * ShablonRadius, Earth.getRasstoyanie() / 25.0f, "Moon", "textures/Sputniks/Earth/moon1.jpg", glm::vec3(5.0f, 1.0f, -5.0f), 20.0f);
    Earth.Sputniks.push_back(Moon);

    Planet Mercury(ShablonRadius * 0.38, Earth.getRasstoyanie() * 0.5, "Mercury", "textures/Planets/mercury1.jpg", glm::vec3(-1.3f, 0.0f, -1.5f), 4.14f, 0.02f * Earth.OsRotateSpeed, 0.0f);

    Planet Venus(ShablonRadius * 0.95, Earth.getRasstoyanie() * 0.7, "Venus", "textures/Planets/venus.jpg", glm::vec3(3.0f, 0.0f, 10.5f), 1.624f, 0.004f * Earth.OsRotateSpeed, 177.3f);

    //Марс со спутниками
    Planet Mars(ShablonRadius * 0.53, Earth.getRasstoyanie() * 1.3, "Mars", "textures/Planets/mars.jpg", glm::vec3(-5.7f, 0.0f, -7.5f), 0.531f, 0.9728f * Earth.OsRotateSpeed, 25.2f);
    Sputnik Phobos(0.06 * ShablonRadius, Mars.getRadius() * 1.8, "Phobos", "textures/Sputniks/Mars/deimos1.jpg", glm::vec3(10.0f, 0.2f, -2.0f), 50.0f);
    Sputnik Deymos(0.03 * ShablonRadius, Mars.getRadius() * 2.2, "Deymos", "textures/Sputniks/Mars/phobos1.jpg", glm::vec3(-5.0f, 0.2f, 10.0f), 20.0f);
    Mars.Sputniks.push_back(Phobos);
    Mars.Sputniks.push_back(Deymos);

    //Юпитер со спутниками
    Planet Jupiter(ShablonRadius * 11.2, Earth.getRasstoyanie() * 4.0, "Jupiter", "textures/Planets/jupiter.jpg", glm::vec3(1.7f, 0.0f, 1.5f), 0.1f, 2.5f * Earth.OsRotateSpeed, 3.1f);
    Sputnik Io(0.3 * ShablonRadius, ShablonRadius * 11.2 * 1.2, "Io", "textures/Sputniks/Jupiter/io1.jpg", glm::vec3(-10.0f, 1.0f, 2.0f), 40.0f);
    Sputnik Callisto(0.5 * ShablonRadius, ShablonRadius * 15.2 * 1.5, "Callisto", "textures/Sputniks/Jupiter/callisto1.jpg", glm::vec3(-5.0f, -1.0f, 10.0f), 20.0f);
    Sputnik Europe(0.7 * ShablonRadius, ShablonRadius * 11.2 * 2.2, "Europe", "textures/Sputniks/Jupiter/europe1.jpg", glm::vec3(-3.0f, 1.5f, 2.0f), 30.0f);
    Sputnik Ganymede(0.8 * ShablonRadius, ShablonRadius * 15.2 * 1.1, "Ganymede", "textures/Sputniks/Jupiter/ganymede1.jpg", glm::vec3(5.0f, -2.0f, 10.0f), 45.0f);
    Jupiter.Sputniks.push_back(Io);
    Jupiter.Sputniks.push_back(Callisto);
    Jupiter.Sputniks.push_back(Europe);
    Jupiter.Sputniks.push_back(Ganymede);

    //Сатурн со спутниками
    Planet Saturn(ShablonRadius * 9.5, Earth.getRasstoyanie() * 7.0, "Saturn", "textures/Planets/saturn1.jpg", glm::vec3(-1.7f, 0.0f, 9.0f), 0.034f, 2.18f * Earth.OsRotateSpeed, 26.7f);
    Sputnik Titan(0.9 * ShablonRadius, Saturn.getRadius() * 2.0, "Titan", "textures/Sputniks/Saturn/titan1.jpg", glm::vec3(-5.0f, 1.8f, 10.0f), 20.0f);
    Sputnik Memas(0.3 * ShablonRadius, Saturn.getRadius() * 1.1, "Memas", "textures/Sputniks/Saturn/mimas1.jpg", glm::vec3(10.0f, 1.8f, 2.0f), 40.0f);
    Saturn.Sputniks.push_back(Memas);
    Saturn.Sputniks.push_back(Titan);

    //Уран со спутниками
    Planet Uranus(ShablonRadius * 3.9, Earth.getRasstoyanie() * 9.0, "Uranus", "textures/Planets/uranus1.jpg", glm::vec3(10.7f, 0.0f, -3.5f), 0.012f, 1.41f * Earth.OsRotateSpeed, 97.8f);
    Sputnik Bianca(0.6 * ShablonRadius, Uranus.getRadius() * 1.6, "Bianca", "textures/Sputniks/Uranus/bianca1.jpg", glm::vec3(-5.0f, 1.6f, 10.0f), 50.0f);
    Sputnik Pak(0.4 * ShablonRadius, Uranus.getRadius() * 1.2, "Pak", "textures/Sputniks/Jupiter/ganymede1.jpg", glm::vec3(10.0f, 0.7f, 2.0f), 40.0f);/*с текстурой Пака что - то не так
                                                                                                                                                    компилятор пишет ошибку на неразрешенную область памяти
                                                                                                                                                    так как внешне Пак похож на Ганимед
                                                                                                                                                    то используем его текстуру*/
    Uranus.Sputniks.push_back(Bianca);
    Uranus.Sputniks.push_back(Pak);

    //Нептун со спутниками
    Planet Neptune(ShablonRadius * 4.0, Earth.getRasstoyanie() * 11.0, "Neptune", "textures/Planets/neptune.jpg", glm::vec3(2.7f, 0.0f, -0.5f), 0.006f, 1.5f * Earth.OsRotateSpeed, 28.3f);
    Sputnik Larissa(0.6 * ShablonRadius, Uranus.getRadius() * 1.3, "Larissa", "textures/Sputniks/Neptune/larissa1.jpg", glm::vec3(-5.0f, 1.6f, 10.0f), 20.0f);
    Neptune.Sputniks.push_back(Larissa);
    List<Asteroid> AsteroidBelt;

    for (int i = 0; i < 2000; ++i) {
        double rasst, height, xpos, speed;
        Asteroid a;
        rasst = rand() % (int)pow(10, 3);
        height = rand() % (int)pow(10, 3);
        xpos = rand() % (int)pow(10, 3);
        speed = rand() % (int)pow(10, 3);
        speed = 10.0 + (speed / pow(10, 3)) * (100.0);
        rasst = Earth.getRasstoyanie() * 2.2 + (rasst / pow(10, 3)) * ((Earth.getRasstoyanie() * 3.6) - (Earth.getRasstoyanie() * 2.2) - 70);
        height = -15.0 + (height / pow(10, 3)) * (30);
        xpos = -rasst + (xpos / pow(10, 3)) * (2 * rasst);
        if (!i) a = Asteroid(10000, rasst, "Астеройд", glm::vec3(xpos, height, rasst), speed, true, 0, 0);
        else a = Asteroid(10000, rasst, "Астеройд", glm::vec3(xpos, height, rasst), speed, false, 0, 0);
        AsteroidBelt.pushElement(a);
    }

    SolarSystem.pushElement(Sun);
    SolarSystem.pushElement(Mercury);
    SolarSystem.pushElement(Venus);
    SolarSystem.pushElement(Earth);
    SolarSystem.pushElement(Mars);
    SolarSystem.pushElement(Jupiter);
    SolarSystem.pushElement(Saturn);
    SolarSystem.pushElement(Uranus);
    SolarSystem.pushElement(Neptune);
    PlanetCoordinates.pushElement(Sun.position);
    PlanetCoordinates.pushElement(Mercury.position);
    PlanetCoordinates.pushElement(Venus.position);
    PlanetCoordinates.pushElement(Earth.position);
    PlanetCoordinates.pushElement(Mars.position);
    PlanetCoordinates.pushElement(Jupiter.position);
    PlanetCoordinates.pushElement(Saturn.position);
    PlanetCoordinates.pushElement(Uranus.position);
    PlanetCoordinates.pushElement(Neptune.position);

    //////////////////////////////////////////////////////////////////////////////////////////
     // Компилируем и устанавливаем шейдер
    Shader textShader("Shaders/textVertexShader.txt", "Shaders/textFragmentShader.txt");
    glm::mat4 projection = glm::perspective(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader.use();
    textShader.setMat4("projection", projection);
    FT_Library ft;

    // Всякий раз, когда возникает ошибка, функции будут возвращать отличное от нуля значение
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }
    // Путь к файлу шрифта
    std::string font_name = "Fonts/arial.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }
    // Загружаем шрифт в face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // Задаем размер для загрузки глифов
        FT_Set_Pixel_Sizes(face, 0, 48);
        // Отключаем ограничение выравнивания байтов
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // Загружаем глифы символов 
        for (unsigned int c = 0; c < 256; c++)
        {
            // Загружаем глиф символа 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // Генерируем текстуру
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // Задаем для текстуры необходимые опции
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Теперь сохраняем символ для последующего использования
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // Освобождаем использованные ресурсы
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 5, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /////////////////////////////////////////////////////////////////////////////////////////////////// Цикл рендеринга
    /*while (!glfwWindowShouldClose(window))*/
    while (!glfwWindowShouldClose(window))
    {
        // Логическая часть работы со временем для каждого кадра
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Обработка ввода
        processInput(window);
        // Рендеринг
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // очищаем буфер цвета и буфер глубины
         //создаем преобразование
        glm::mat4 view = glm::mat4(1.0f); // сначала инициализируем единичную матрицу
        view = camera.GetLookAtMatrix();
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 15000.0f);
        if (ChoosenPlanet != 999) {//если нажата кнопка перемещения к планете
            textShader.use();
            //// передаём матрицы преобразования в шейдеры
            textShader.setMat4("projection", projection);
            textShader.setMat4("view", view);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(SolarSystem[ChoosenPlanet].position.x - 0.5f * SolarSystem[ChoosenPlanet].getRadius(), SolarSystem[ChoosenPlanet].position.y + 1.3 * SolarSystem[ChoosenPlanet].getRadius(), SolarSystem[ChoosenPlanet].position.z));
            textShader.setMat4("model", model);
            if (!HidePlanetNames)
                RenderText(textShader, SolarSystem[ChoosenPlanet].getName(), 0.0f, 0.0f, 0.0f, 0.008f * SolarSystem[ChoosenPlanet].getRadius(), glm::vec3(1.0f, 1.0f, 1.0f));//рисуем имя планеты
            //рисуем имена спутников планеты
            for (int i = 0; i < SolarSystem[ChoosenPlanet].Sputniks.size(); ++i) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(SolarSystem[ChoosenPlanet].Sputniks[i].position.x - 1.55f * SolarSystem[ChoosenPlanet].Sputniks[i].getRadius(), SolarSystem[ChoosenPlanet].Sputniks[i].position.y + 1.2 * SolarSystem[ChoosenPlanet].Sputniks[i].getRadius(), SolarSystem[ChoosenPlanet].Sputniks[i].position.z));
                textShader.setMat4("model", model);
                if (!HidePlanetNames)
                    RenderText(textShader, SolarSystem[ChoosenPlanet].Sputniks[i].getName(), 0.0f, 0.0f, 0.0f, 0.03f * SolarSystem[ChoosenPlanet].Sputniks[i].getRadius(), glm::vec3(1.0f, 1.0f, 1.0f));
            }
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        meteorShader.use(); // не забудьте активировать шейдер перед настройкой uniform-переменных!  
        meteorShader.setMat4("view", view);
        meteorShader.setMat4("projection", projection);
        glBindVertexArray(VAO1);
        for (int i = 0; i < AsteroidBelt.length(); ++i) {
            AsteroidBelt[i].draw((float)glfwGetTime(), meteorShader, SpeedMultiplier);
        }
        glBindVertexArray(0);
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        PlanetShader.use(); // не забываем активировать шейдер перед настройкой uniform-переменных!  
        glm::vec3 positionRings;
        PlanetShader.setMat4("view", view);
        PlanetShader.setMat4("projection", projection);
        glBindVertexArray(VAO);
        float k = (float)glfwGetTime();
        for (unsigned int i = 0; i < SolarSystem.length(); i++)
        {
            SolarSystem[i].draw(k, PlanetShader, StopPlanetsMoving, SpeedMultiplier);
            if (i == 6) {
                positionRings = glm::vec3(SolarSystem[i].position.x, SolarSystem[i].position.y, SolarSystem[i].position.z);
            }
            PlanetCoordinates[i] = glm::vec3(SolarSystem[i].position.x, SolarSystem[i].position.y, SolarSystem[i].position.z);
        }
        glBindVertexArray(0);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ringShader.use(); // не забываем активировать шейдер перед настройкой uniform-переменных!
        // передаём матрицы преобразования в шейдеры
        ringShader.setMat4("view", view);
        ringShader.setMat4("projection", projection);
        glBindVertexArray(ringVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, positionRings);
        model = glm::rotate(model, (float)glfwGetTime() * 5, glm::vec3(0.0f, 1.0f, 0.0f));
        ringShader.setMat4("model", model);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, (majorRadius - minorRadius) * segmentsCirc * 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (!HideTrajectories) {//если пользователь не захотел прятать траектории
            torusShader.use(); // не забываем активировать шейдер перед настройкой uniform-переменных
             // передаём матрицы преобразования в шейдеры
            torusShader.setMat4("view", view);
            torusShader.setMat4("projection", projection);
            glBindVertexArray(torusVAO);
            for (int i = 0; i < CoeffsTorusRadius.length(); ++i) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::scale(model, glm::vec3(CoeffsTorusRadius[i], 1.0f, CoeffsTorusRadius[i]));
                torusShader.setMat4("model", model);
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDrawElements(GL_TRIANGLES, torusCircNum * segmentsCirc * 6, GL_UNSIGNED_INT, 0);
            }
            glBindVertexArray(0);
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        glDepthFunc(GL_LEQUAL); // меняем функцию глубины, чтобы обеспечить прохождение теста глубины, когда значения равны содержимому буфера глубины
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetLookAtMatrix())); // убираем из матрицы вида секцию, отвечающую за операцию трансляции
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        //Куб скайбокса
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // восстанавливаем стандартное значение функции теста глубины
        // glfw: обмен содержимым front- и back- буферов. Отслеживание событий ввода/вывода (была ли нажата/отпущена кнопка, перемещен курсор мыши и т.п.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Опционально: освобождаем все ресурсы, как только они выполнили свое предназначение
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO1);
    glDeleteBuffers(1, &EBO1);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &skyboxVBO);
    glDeleteVertexArrays(1, &ringVAO);
    glDeleteVertexArrays(1, &ringVBO);
    glDeleteBuffers(1, &ringEBO);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteVertexArrays(1, &textVBO);
    // glfw: завершение, освобождение всех выделенных ранее GLFW-реурсов
    glfwTerminate();
    return 0;
}

int countClac = 0;//счетчик нажатия на клавишу 'O'
int countUp = 0;
int countDown = 0;

// Обработка всех событий ввода: запрос GLFW о нажатии/отпускании кнопки мыши в данном кадре и соответствующая обработка данных событий
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)//движение вперед
        camera.CameraKeyBoardMoving(1, deltaTime, firstMouseMoving);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)//движение назад
        camera.CameraKeyBoardMoving(2, deltaTime, firstMouseMoving);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)//движение влево
        camera.CameraKeyBoardMoving(3, deltaTime, firstMouseMoving);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)//движение вправо
        camera.CameraKeyBoardMoving(4, deltaTime, firstMouseMoving);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)//движение вверх
        camera.CameraKeyBoardMoving(5, deltaTime, firstMouseMoving);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)//движение вниз
        camera.CameraKeyBoardMoving(6, deltaTime, firstMouseMoving);
    ////////////навигация по планетам (номер клавиши - соответствующая ей планета)
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
        camera.lookAtPlanet(PlanetCoordinates[0], SolarSystem[0].getRadius());
        StopPlanetsMoving = 1;
        ChoosenPlanet = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        camera.lookAtPlanet(PlanetCoordinates[1], SolarSystem[1].getRadius());
        StopPlanetsMoving = 0;
        ChoosenPlanet = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        camera.lookAtPlanet(PlanetCoordinates[2], SolarSystem[2].getRadius());
        StopPlanetsMoving = 0;
        ChoosenPlanet = 2;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        camera.lookAtPlanet(PlanetCoordinates[3], SolarSystem[3].getRadius());
        StopPlanetsMoving = 0;
        ChoosenPlanet = 3;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        camera.lookAtPlanet(PlanetCoordinates[4], SolarSystem[4].getRadius());
        StopPlanetsMoving = 0;
        ChoosenPlanet = 4;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        camera.lookAtPlanet(PlanetCoordinates[5], SolarSystem[5].getRadius());
        StopPlanetsMoving = 0;
        ChoosenPlanet = 5;
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        camera.lookAtPlanet(PlanetCoordinates[6], SolarSystem[6].getRadius());
        StopPlanetsMoving = 0;
        ChoosenPlanet = 6;
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        camera.lookAtPlanet(PlanetCoordinates[7], SolarSystem[7].getRadius());
        StopPlanetsMoving = 0;
        ChoosenPlanet = 7;
    }
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
        camera.lookAtPlanet(PlanetCoordinates[8], SolarSystem[8].getRadius());
        StopPlanetsMoving = 0;
        ChoosenPlanet = 8;
    }
    //вернуться из обзора по планетам к дефолтному обзору, как при запуске приложения
    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        camera.getBaseLook();
        StopPlanetsMoving = 1;
        ChoosenPlanet = 999;
    }
    //кнопка скрытия траекторий движения планет
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        ++countClac;
        if ((countClac % 7 == 0)) {
            if (!HideTrajectories)HideTrajectories = true;
            else HideTrajectories = false;
            countClac = 0;
        }
    }
    //кнопка скрытия траекторий движения планет
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        ++countClac;
        if ((countClac % 7 == 0)) {
            if (!HidePlanetNames)HidePlanetNames = true;
            else HidePlanetNames = false;
            countClac = 0;
        }
    }
    //кнопка скрытия траекторий движения планет
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        ++countClac;
        if ((countClac % 10 == 0)) {
            if (!HideTrajectories)HideTrajectories = true;
            else HideTrajectories = false;
            countClac = 0;
        }
    }
    //кнопка увеличения скорости движения планет в 2 раза
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        ++countUp;
        if (countUp % 10 == 0) {
            if (SpeedMultiplier == 16.0)return;
            else SpeedMultiplier *= 2;
            countUp = 0;
        }
    }
    //кнопка уменьшения скорости движения планет в 2 раза
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        ++countDown;
        if (countDown % 10 == 0) {
            if (SpeedMultiplier == 0.125)return;
            else SpeedMultiplier /= 2;
            countDown = 0;
        }
    }
}

bool lbutton_down = false;
double xposBeg, yposBeg;
bool mouseMoveAfterClick = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // перевернуто, так как y-координаты идут снизу вверх
    if (!lbutton_down) {
        return;
    }
    if (firstMouseMoving)
    {
        float xoffset = (xpos - xposBeg);
        float yoffset = (yposBeg - ypos);
        camera.ControlMouseMoving(xoffset, yoffset, true);
        firstMouseMoving = false;
    }
    else {
        if (lbutton_down) {
            if (mouseMoveAfterClick) {
                float xoffset = (xpos - xposBeg);
                float yoffset = (yposBeg - ypos);
                lastX = xpos;
                lastY = ypos;
                mouseMoveAfterClick = false;
                camera.ControlMouseMoving(xoffset, yoffset, false);
            }
            else {
                float xoffset = (xpos - lastX);
                float yoffset = (lastY - ypos);
                lastX = xpos;
                lastY = ypos;
                camera.ControlMouseMoving(xoffset, yoffset, false);
            }
        }
    }
}

void mousePress_callback(GLFWwindow* window, int button, int action, int mods) {
    // перевернуто, так как y-координаты идут снизу вверх
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (GLFW_PRESS == action) {
            lbutton_down = true;
            mouseMoveAfterClick = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xposBeg, &yposBeg);
            glfwSetCursorPosCallback(window, mouse_callback);
        }
        else if (GLFW_RELEASE == action)
            lbutton_down = false;
    }
}

// glfw: всякий раз, когда изменяются размеры окна (пользователем или операционной системой), вызывается данная callback-функция
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Убеждаемся, что окно просмотра соответствует новым размерам окна.
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


unsigned int loadCubemap(vector<string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}



void RenderText(Shader& shader, std::string text, float x, float y, float z, float scale, glm::vec3 color)
{
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);
    // Перебираем все символы
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        shader.setVec3("textColor", color);

        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[] = {
            // координаты       // текстурные координаты
            xpos + w,  ypos + h,  0.0f,    1.0f, 1.0f, // верхняя правая вершина
            xpos + w,  ypos,  0.0f,   1.0f, 0.0f, // нижняя правая вершина
            xpos ,  ypos ,    0.0f,      0.0f, 0.0f, // нижняя левая вершина
            xpos ,  ypos + h, 0.0f,   0.0f, 1.0f  // верхняя левая вершина
        };
        unsigned int indices[] = {
            0, 1, 3, // первый треугольник
            1, 2, 3  // второй треугольник
        };
        // Рендерим текстуру глифа на прямоугольник
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Обновляем содержимое памяти VBO
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // используем glBufferSubData вместо glBufferData так как используем GL_DYNAMIC_DRAW
        GLuint textEBO;
        glGenBuffers(1, &textEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Рендер прямоугольника
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Теперь производим смещение для отображения следующего глифа (обратите внимание, что данное смещение измеряется в единицах, составляющих 1/64 пикселя)
        x += (ch.Advance >> 6) * scale; // битовый сдвиг на 6 разрядов, чтобы получить значение в пикселях (2^6 = 64 (разделите количество 1/64-х пикселей на 64, чтобы получить количество пикселей))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}