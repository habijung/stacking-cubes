//
// Created by habi on 2023-08-21.
//

#define GLFW_INCLUDE_NONE
#define STB_IMAGE_IMPLEMENTATION

#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "include/objects.h"
#include "include/shader.h"
#include "include/stb_image.h"
#include <iostream>
using namespace std;
using namespace glm;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double px, double py);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
unsigned int load_texture(const char *img);

// Create camera
vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;
const float SCR_RATE = (float) SCR_WIDTH / (float) SCR_HEIGHT;

float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
float _yaw = -90.0f;
float _pitch = 0.0f;
float fov = 45.0f;
bool firstMouse = true;
bool showCursor = true;

float deltaTime = 0.0f;// Time between current and last frame
float lastFrame = 0.0f;// Time of last frame

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Cube Maker", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Shader
    // TODO: 어디에 사용되는 shader인지 바로 알기 위해 shader 변수 이름을 object 이름으로 변경하기
    Shader shaderWall("include/wall.vert", "include/wall.frag");
    Shader shaderWood("include/wood.vert", "include/wood.frag");
    Shader shaderOutline("include/wood.vert", "include/outline.frag");

    // Texture
    unsigned int textureWall = load_texture("include/wall.jpg");
    unsigned int textureWood = load_texture("include/wood.jpg");

    // Object: Plane
    unsigned int planeVAO, planeVBO, planeEBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glGenBuffers(1, &planeEBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

    // Object: Cube
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shaderWall.use();
    shaderWall.setInt("textureWall", 0);
    shaderWood.use();
    shaderWood.setInt("textureWood", 0);

    // Rendering
    while (!glfwWindowShouldClose(window)) {
        // Calculate frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Rendering somethings
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Object: Plane
        // Camera, View transformation
        mat4 view, model, projection = mat4(1.0f);
        view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        projection = perspective(radians(fov), SCR_RATE, 0.1f, 100.0f);

        shaderWall.use();
        shaderWall.setMat4("view", view);
        shaderWall.setMat4("projection", projection);

        // Draw
        glBindVertexArray(planeVAO);
        glBindTexture(GL_TEXTURE_2D, textureWall);// Default texture: GL_TEXTURE0
        model = mat4(1.0f);
        model = translate(model, vec3(-1.5f, 0.0f, 0.0f));
        shaderWall.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Object: Cube
        shaderWood.use();
        shaderWood.setMat4("view", view);
        shaderWood.setMat4("projection", projection);

        // Draw
        glBindVertexArray(cubeVAO);
        glBindTexture(GL_TEXTURE_2D, textureWood);
        model = mat4(1.0f);
        model = translate(model, vec3(1.0f, 0.0f, 0.0f));
        // model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));
        shaderWood.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Optional
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeEBO);

    glfwTerminate();
    return 0;
}


void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    // Quit program
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Camera position
    float cameraSpeed = static_cast<float>(2.5f * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= normalize(cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += normalize(cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

void mouse_callback(GLFWwindow *window, double px, double py) {
    float xpos = static_cast<float>(px);
    float ypos = static_cast<float>(py);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;// reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Mouse cursor off이면 _yaw, _pitch를 고정시켜서 화면을 움직이게 하지 않음.
    if (!showCursor) {
        _yaw += xoffset;
        _pitch += yoffset;
    }

    if (_pitch > 89.0f) {
        _pitch = 89.0f;
    }
    if (_pitch < -89.0f) {
        _pitch = -89.0f;
    }

    vec3 direction;
    direction.x = cos(radians(_yaw)) * cos(radians(_pitch));
    direction.y = sin(radians(_pitch));
    direction.z = sin(radians(_yaw)) * cos(radians(_pitch));
    cameraFront = normalize(direction);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    fov -= (float) yoffset;// Field of View (or Angle of View)
    if (fov < 1.0f) {
        fov = 1.0f;
    }
    if (fov > 45.0f) {
        fov = 45.0f;
    }
}

unsigned int load_texture(const char *img) {
    unsigned int tex;
    glGenTextures(1, &tex);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(img, &width, &height, &nrChannels, 0);

    if (data) {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set the texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        cout << "Failed to load texture" << endl;
    }

    return tex;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // Mouse cursor mode
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        if (showCursor) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetCursorPos(window, SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        showCursor ^= 1;
    }
}
