#define GLFW_DLL
#define GLEW_DLL

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Model.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"


glm::vec3 cameraPosition = glm::vec3(15.0, 12.0, 15.0);
glm::vec3 cameraTarget = glm::vec3(0.0f);         
glm::vec3 cameraFront = glm::normalize(cameraTarget - cameraPosition);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

float yaw = glm::degrees(atan2(cameraFront.z, cameraFront.x));
float pitch = glm::degrees(asin(cameraFront.y));
float lastX = 256.0f;
float lastY = 256.0f;
bool firstMouse = true;

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = 0.01f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;


    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;


    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR GLFW INIT: \n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "MainWindow", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    GLenum ret = glewInit();
    if (GLEW_OK != ret) {
        fprintf(stderr, "GLEW error: %s\n", glewGetErrorString(ret));
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader labShader("vertex_shader.glsl", "fragment_shader.glsl");

    Model ourModel("model.obj");

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    glm::vec3 lightPos(5.0f, 5.0f, 10.0f);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        labShader.activate();

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

        labShader.setMat4("projection", projection);
        labShader.setMat4("view", view);
        labShader.setMat4("model", model);
        labShader.setMat4("transform", glm::mat4(1.0f));
        labShader.setMat3("normalMatrix", normalMatrix);

        labShader.setVec3("viewPos", cameraPosition);

        labShader.setVec3("light.position", lightPos);
        // Фоновый (ambient) — слабая интенсивность
        labShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        // Диффузный (diffuse) — основной цвет освещения
        labShader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
        // Зеркальный (specular) — полная интенсивность
        labShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // Цвет поверхности — тёплый медно-оранжевый
        labShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
        labShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
        labShader.setVec3("material.specular", 0.5f, 0.5f, 0.50f);
        labShader.setFloat("material.shininess", 32.0f);

        ourModel.Draw(labShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}




