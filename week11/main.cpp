// 11-апта: суретті файлдан жүктеу, текстура объектісі және оның параметрлері
//
// 10-аптадағы еркін камера (W A S D, тінтуір, дөңгелек) сақталған.
// Басқару: W/A/S/D жылжу, Space/Left Shift жоғары/төмен, Left Ctrl жүгіру,
//          тінтуір - бұрылу, дөңгелек - zoom, Esc - шығу
//   1 REPEAT   2 MIRRORED_REPEAT   3 CLAMP_TO_EDGE   4 CLAMP_TO_BORDER  (1-тапсырма)
//   N / L      MAG сүзгісі: NEAREST / LINEAR                            (2-тапсырма)
//   M          MIN сүзгісі: LINEAR (mipmap жоқ) <-> LINEAR_MIPMAP_LINEAR (3-тапсырма)
//   [ ]        uvScale 0.5..8.0 (dt-ге көбейтіледі)                     (5-тапсырма)
//   T tint (Қ2)   F ағып жатқан су (Қ3)   Z анизотропты сүзгі (Қ4)
//
// Тексеру флагтары (скриншоттарды терминалдан алу үшін):
//   --wrap repeat|mirror|edge|border   --mag nearest|linear|bad   --min linear|mipmap|nomip
//   --uv-mul f   vertices[] ішіндегі барлық UV-ді f-ке көбейтеді (1-тапсырма: 2.0)
//   --far        z = -40 маңына 6 куб қосады (3-тапсырма)
//   --floor      үлкен жалпақ куб (Қ4)     --aniso   анизотропты сүзгіні қосу
//   --cam x,y,z  --yaw a  --pitch a  --fov a   --tex файл (Қ1: өз суретің)
//   --shot f.ppm 10-кадрдан кейін скриншот сақтап шығу
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 1-қадам: stb_image. #define ӘРҚАШАН #include-тан бұрын тұруы керек,
// әйтпесе "undefined reference to stbi_load" шығады.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#ifndef ASSET_DIR
#define ASSET_DIR "assets"
#endif

// 3-қадам: позиция (3) + UV (2). Әр жақ: (0,0) (1,0) (1,1) / (0,0) (1,1) (0,1),
// сыртынан қарағанда CCW (10-аптадағы позициялар өзгермеген).
float vertices[] = {
    // +Z алдыңғы
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    // -Z артқы
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    // -X сол
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    // +X оң
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    // +Y жоғары
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    // -Y төмен
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
};

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,   0.0f),
    glm::vec3( 2.0f,  0.5f,  -2.0f),
    glm::vec3(-1.8f, -0.3f,  -1.0f),
    glm::vec3( 1.2f, -0.8f,  -3.5f),
    glm::vec3(-2.2f,  1.0f,  -4.0f),
};
// 3-тапсырма: z = -40 маңындағы алыс кубтар (--far)
glm::vec3 farPositions[] = {
    glm::vec3( 0.0f,  0.0f, -40.0f),
    glm::vec3( 3.0f,  1.0f, -42.0f),
    glm::vec3(-3.0f, -1.0f, -41.0f),
    glm::vec3( 6.0f,  0.0f, -44.0f),
    glm::vec3(-6.0f,  1.5f, -43.0f),
    glm::vec3( 1.5f, -2.0f, -45.0f),
};

// ---- Камера (9-10 апта) ----
glm::vec3 camPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp    = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f, pitch = 0.0f, fov = 45.0f;
float lastX = 400.0f, lastY = 300.0f;
bool  firstMouse = true;
const int gWidth = 800, gHeight = 600;

// ---- Текстура параметрлері (ауыстырылады) ----
GLenum wrapMode  = GL_REPEAT;
GLenum magFilter = GL_LINEAR;
GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR;
float  uvScale   = 1.0f;       // 5-тапсырма: шейдердегі UV масштабы
bool   tintOn = false, flowOn = false, anisoOn = false;
bool   drawFar = false, drawFloor = false;
unsigned int gTexture = 0;

GLFWwindow* gWindow = nullptr;

void updateFront() {
    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(dir);
}

void mouse_callback(GLFWwindow*, double xpos, double ypos) {
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos; lastY = (float)ypos;
    yaw += xoffset * 0.1f;
    pitch += yoffset * 0.1f;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    updateFront();
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 90.0f) fov = 90.0f;
}

// ---- Текстура параметрлерін қолдану. Бұл функция байланған текстураға әсер етеді ----
void applyTexParams() {
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    // 1-тапсырма кеңесі: CLAMP_TO_BORDER үшін шекара түсі бөлек беріледі (қызыл)
    float border[] = {1.0f, 0.0f, 0.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    // Қ4: анизотропты сүзгі (кеңейтім)
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
    if (GLEW_EXT_texture_filter_anisotropic) {
        float maxAniso = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoOn ? maxAniso : 1.0f);
    }
#endif
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) std::fprintf(stderr, "glGetError після параметрлер: 0x%04X\n", err);
}

void processInput(GLFWwindow* w, float dt) {
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(w, true);

    float speed = 2.5f * dt;
    if (glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) speed *= 2.0f;
    glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
    if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) camPos += speed * camFront;
    if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) camPos -= speed * camFront;
    if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) camPos -= right * speed;
    if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) camPos += right * speed;
    if (glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS) camPos += speed * glm::vec3(0, 1, 0);
    if (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camPos -= speed * glm::vec3(0, 1, 0);

    // 5-тапсырма: [ ] - үздіксіз әрекет, сондықтан dt-ге көбейтіледі
    if (glfwGetKey(w, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) uvScale += 2.0f * dt;
    if (glfwGetKey(w, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)  uvScale -= 2.0f * dt;
    if (uvScale < 0.5f) uvScale = 0.5f;
    if (uvScale > 8.0f) uvScale = 8.0f;

    // Режим ауыстырғыштар - бір басуға бір рет (алдыңғы күйді сақтаймыз)
    static bool prev[512] = {};
    auto pressed = [&](int key) {
        bool now = glfwGetKey(w, key) == GLFW_PRESS;
        bool hit = now && !prev[key];
        prev[key] = now;
        return hit;
    };
    bool changed = false;
    if (pressed(GLFW_KEY_1)) { wrapMode = GL_REPEAT; changed = true; }
    if (pressed(GLFW_KEY_2)) { wrapMode = GL_MIRRORED_REPEAT; changed = true; }
    if (pressed(GLFW_KEY_3)) { wrapMode = GL_CLAMP_TO_EDGE; changed = true; }
    if (pressed(GLFW_KEY_4)) { wrapMode = GL_CLAMP_TO_BORDER; changed = true; }
    if (pressed(GLFW_KEY_N)) { magFilter = GL_NEAREST; changed = true; }
    if (pressed(GLFW_KEY_L)) { magFilter = GL_LINEAR; changed = true; }
    if (pressed(GLFW_KEY_M)) {
        minFilter = (minFilter == GL_LINEAR_MIPMAP_LINEAR) ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR;
        changed = true;
    }
    if (pressed(GLFW_KEY_Z)) { anisoOn = !anisoOn; changed = true; }
    if (pressed(GLFW_KEY_T)) tintOn = !tintOn;
    if (pressed(GLFW_KEY_F)) flowOn = !flowOn;
    if (changed) {
        applyTexParams();
        std::printf("wrap=0x%X mag=0x%X min=0x%X aniso=%d\n", wrapMode, magFilter, minFilter, (int)anisoOn);
    }
}

// 5-қадам: шейдерлер. texture() - 3.3 Core синтаксисі (texture2D емес!).
const char* vsSrc = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vTexCoord = aTexCoord;
})";

const char* fsSrc = R"(#version 330 core
in  vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D ourTexture;
uniform float uvScale;   // 5-тапсырма
uniform float time;      // Қ3
uniform float flow;      // Қ3: 0 немесе 1
uniform vec3  tint;      // Қ2
void main() {
    vec2 uv = vTexCoord * uvScale + vec2(0.0, flow * time * 0.1);
    FragColor = texture(ourTexture, uv) * vec4(tint, 1.0);
})";

GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, 1024, nullptr, log);
        std::fprintf(stderr, "Shader error: %s\n", log);
    }
    return s;
}

// 6-қадам: текстураны жүктеу
unsigned int loadTexture(const char* path, bool generateMipmaps) {
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    // (параметрлер applyTexParams() ішінде беріледі; бастапқы мәндер осында да қойылады)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);  // OpenGL-де (0,0) - төменгі сол жақ
    int w, h, n;
    unsigned char* data = stbi_load(path, &w, &h, &n, 0);
    if (data) {
        GLenum fmt = (n == 4) ? GL_RGBA : (n == 1 ? GL_RED : GL_RGB);
        // 3 арналы суретте жол ұзындығы 4-ке еселі болмауы мүмкін
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        if (generateMipmaps) glGenerateMipmap(GL_TEXTURE_2D);  // glTexImage2D-ден КЕЙІН
        std::cout << path << "  " << w << "x" << h << "  арна: " << n << std::endl;
    } else {
        std::cerr << "Сурет жүктелмеді: " << path << std::endl;
    }
    stbi_image_free(data);
    return id;
}

void saveShot(const char* path, int w, int h) {
    std::vector<unsigned char> px(w * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    FILE* f = std::fopen(path, "wb");
    if (!f) return;
    std::fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (int y = h - 1; y >= 0; y--) std::fwrite(&px[y * w * 3], 1, w * 3, f);
    std::fclose(f);
}

int main(int argc, char** argv) {
    const char* shotPath = nullptr;
    std::string texPath = std::string(ASSET_DIR) + "/container.png";  // 2-жиі қате: қысқа жол жазбау
    float uvMul = 1.0f, fixedDt = 0.0f;
    bool noMipmapGen = false;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        auto next = [&]() -> std::string { return i + 1 < argc ? argv[++i] : ""; };
        if (a == "--shot") shotPath = argv[++i];
        else if (a == "--uv-mul") uvMul = (float)std::atof(next().c_str());
        else if (a == "--fixed-dt") fixedDt = (float)std::atof(next().c_str());
        else if (a == "--far") drawFar = true;
        else if (a == "--floor") drawFloor = true;
        else if (a == "--aniso") anisoOn = true;
        else if (a == "--tex") texPath = next();
        else if (a == "--fov") fov = (float)std::atof(next().c_str());
        else if (a == "--yaw") yaw = (float)std::atof(next().c_str());
        else if (a == "--pitch") pitch = (float)std::atof(next().c_str());
        else if (a == "--cam") std::sscanf(next().c_str(), "%f,%f,%f", &camPos.x, &camPos.y, &camPos.z);
        else if (a == "--uvscale") uvScale = (float)std::atof(next().c_str());
        else if (a == "--wrap") {
            std::string v = next();
            wrapMode = v == "mirror" ? GL_MIRRORED_REPEAT : v == "edge" ? GL_CLAMP_TO_EDGE
                     : v == "border" ? GL_CLAMP_TO_BORDER : GL_REPEAT;
        } else if (a == "--mag") {
            std::string v = next();
            magFilter = v == "nearest" ? GL_NEAREST : v == "bad" ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        } else if (a == "--min") {
            std::string v = next();
            minFilter = v == "linear" ? GL_LINEAR : v == "nomip" ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_LINEAR;
            if (v == "nomip") noMipmapGen = true;  // 6-жиі қате: MIN mipmap, бірақ glGenerateMipmap жоқ
        }
    }

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow* window = glfwCreateWindow(gWidth, gHeight, "Week 11 - Textures", nullptr, nullptr);
    if (!window) { std::fprintf(stderr, "Window error\n"); glfwTerminate(); return -1; }
    gWindow = window;
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::fprintf(stderr, "GLEW error\n"); return -1; }
    glGetError();  // glewInit-тің GL_INVALID_ENUM қалдығын тазалау

    if (!shotPath) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    GLuint vs = compile(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsSrc);
    GLuint shader = glCreateProgram();
    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glLinkProgram(shader);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // 1-тапсырма: --uv-mul болса, UV мәндерін GPU-ға жібермес бұрын көбейтеміз
    const int count = sizeof(vertices) / sizeof(float);
    std::vector<float> data(vertices, vertices + count);
    for (int i = 0; i < count; i += 5) { data[i + 3] *= uvMul; data[i + 4] *= uvMul; }

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    // 4-қадам: stride 5 (екі жерде де!), UV - 2 компонент
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    gTexture = loadTexture(texPath.c_str(), !noMipmapGen);
    applyTexParams();
    updateFront();

    int locModel = glGetUniformLocation(shader, "model");
    int locView  = glGetUniformLocation(shader, "view");
    int locProj  = glGetUniformLocation(shader, "projection");
    int locTex   = glGetUniformLocation(shader, "ourTexture");
    int locScale = glGetUniformLocation(shader, "uvScale");
    int locTime  = glGetUniformLocation(shader, "time");
    int locFlow  = glGetUniformLocation(shader, "flow");
    int locTint  = glGetUniformLocation(shader, "tint");

    float lastTime = (float)glfwGetTime();
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        float now = (float)glfwGetTime();
        float dt = fixedDt > 0.0f ? fixedDt : now - lastTime;
        lastTime = now;
        processInput(window, dt);

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        // 7-қадам: render циклінде байлау (әдетті қазірден бастаймыз - 12-аптада екі текстура)
        glUniform1i(locTex, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gTexture);

        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)gWidth / gHeight, 0.1f, 200.0f);
        glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1f(locTime, now);
        glUniform1f(locFlow, flowOn ? 1.0f : 0.0f);
        // Қ2: tint уақыт бойынша өзгереді
        glm::vec3 tint = tintOn ? glm::vec3(0.6f + 0.4f * sin(now), 0.6f + 0.4f * sin(now + 2.1f),
                                            0.6f + 0.4f * sin(now + 4.2f))
                                : glm::vec3(1.0f);
        glUniform3fv(locTint, 1, glm::value_ptr(tint));

        glBindVertexArray(vao);
        glUniform1f(locScale, uvScale);
        auto drawCube = [&](glm::vec3 pos, int i, float rotate) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            if (rotate > 0.0f) model = glm::rotate(model, now * (0.5f + i * 0.3f) * rotate, glm::vec3(0.3f, 1.0f, 0.0f));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        };
        // --shot режимінде кубтар айналмайды, скриншот қайталанатын болсын
        float rot = shotPath ? 0.0f : 1.0f;
        for (int i = 0; i < 5; i++) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
            model = glm::rotate(model, shotPath ? 0.4f * (i + 1) : now * (0.5f + i * 0.3f), glm::vec3(0.3f, 1.0f, 0.0f));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        if (drawFar) for (int i = 0; i < 6; i++) drawCube(farPositions[i], i, rot);
        if (drawFloor) {  // Қ4: еденді имитациялайтын жалпақ куб
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, -20.0f));
            model = glm::scale(model, glm::vec3(30.0f, 0.1f, 60.0f));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1f(locScale, uvScale * 12.0f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glUniform1f(locScale, uvScale);
        }

        frame++;
        if (shotPath && frame == 10) { saveShot(shotPath, gWidth, gHeight); break; }
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}
