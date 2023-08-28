//
// Created by habi on 2023-08-21.
//

#define GLFW_INCLUDE_NONE
#define STB_IMAGE_IMPLEMENTATION
#define BUFSIZE 512

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
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void processHits(GLint hits, GLuint buffer[]);
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

struct PixelInfo {
    uint ObjectID = 0;
    uint DrawID = 0;
    uint PrimID = 0;

    void Print() {
        printf("Object %d draw %d prim %d\n", ObjectID, DrawID, PrimID);
    }
};
unsigned int framebuffer;


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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

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

    Shader screenShader("include/fbo.vert", "include/fbo.frag");
    float quadVertices[] = {// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                            // positions   // texCoords
                            -1.0f, 1.0f, 0.0f, 1.0f,
                            -1.0f, -1.0f, 0.0f, 0.0f,
                            1.0f, -1.0f, 1.0f, 0.0f,

                            -1.0f, 1.0f, 0.0f, 1.0f,
                            1.0f, -1.0f, 1.0f, 0.0f,
                            1.0f, 1.0f, 1.0f, 1.0f};
    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

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

    screenShader.use();
    screenShader.setInt("screenTexture", 0);
    // framebuffer configuration
    // -------------------------
    //    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);          // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);// now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Rendering
    while (!glfwWindowShouldClose(window)) {
        // Calculate frame time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Rendering somethings
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
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
        glPushName(1);
        glBindVertexArray(planeVAO);
        glBindTexture(GL_TEXTURE_2D, textureWall);// Default texture: GL_TEXTURE0
        model = mat4(1.0f);
        model = translate(model, vec3(-1.5f, 0.0f, 0.0f));
        shaderWall.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glPopName();
        glBindVertexArray(0);

        // Object: Cube
        shaderWood.use();
        shaderWood.setMat4("view", view);
        shaderWood.setMat4("projection", projection);

        // Draw
        glPushName(2);
        glBindVertexArray(cubeVAO);
        glBindTexture(GL_TEXTURE_2D, textureWood);
        model = mat4(1.0f);
        model = translate(model, vec3(1.0f, 0.0f, 0.0f));
        // model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));
        shaderWood.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glPopName();
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);

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

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &rbo);

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

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        cout << "Click: (" << xpos << ", " << ypos << ")" << endl;

//        PixelInfo Pixel;
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
//        glReadBuffer(GL_COLOR_ATTACHMENT0);
//        glReadPixels(xpos, SCR_HEIGHT - ypos, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, &Pixel);
//        glReadBuffer(GL_NONE);
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
//        Pixel.Print();
        GLint hits;
        hits = glRenderMode(GL_RENDER);
        cout << hits << endl;
    }
}

void processHits(GLint hits, GLuint buffer[])
{
    unsigned int i, j;
    GLuint names, *ptr;

    printf("hits = %d\n", hits);
    ptr = (GLuint *) buffer;
    for (i = 0; i < hits; i++) {  /* for each hit  */
        names = *ptr;
        printf(" number of names for hit = %d\n", names); ptr++;
        printf("  z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
        printf(" z2 is %g\n", (float) *ptr/0x7fffffff); ptr++;
        printf("   the name is ");
        for (j = 0; j < names; j++) {  /* for each name */
            printf("%d ", *ptr); ptr++;
        }
        printf("\n");
    }
}

