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

// Create camera
vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

float lastX = 400;
float lastY = 300;
float _yaw = -90.0f;
float _pitch = 0.0f;
float fov = 45.0f;
bool firstMouse = true;

float deltaTime = 0.0f;// Time between current and last frame
float lastFrame = 0.0f;// Time of last frame

bool showCube = false;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Cube Maker", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

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

    // Shader
    const char *vert = "include/shader.vert";
    const char *frag = "include/shader.frag";
    Shader ourShader(vert, frag);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set the texture wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    const char *img = "include/wall.jpg";
    unsigned char *data = stbi_load(img, &width, &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        cout << "Failed to load texture" << endl;
    }
    stbi_image_free(data);

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Projection matrix rarely changes there's no need to do this per frame
    ourShader.use();
    mat projection = perspective(radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);

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

        // Render container
        ourShader.use();

        // Camera, View transformation
        mat4 view = mat4(1.0f);
        view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("view", view);

        // Draw object: Plane
        glBindVertexArray(planeVAO);
        glBindTexture(GL_TEXTURE_2D, texture);
        mat4 model = mat4(1.0f);
        model = translate(model, vec3(-1.5f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw object: Cube
        if (showCube) {
            glBindVertexArray(cubeVAO);
            glBindTexture(GL_TEXTURE_2D, texture);
            model = mat4(1.0f);
            model = translate(model, vec3(1.0f, 0.0f, 0.0f));
            // model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));
            ourShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

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

    // Create cube
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
        showCube = false;
    }
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
        showCube = true;
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
    _yaw += xoffset;
    _pitch += yoffset;

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
