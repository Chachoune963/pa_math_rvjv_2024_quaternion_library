#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include "library.h"

const GLint WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;

// NOTE: Vertex Shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;
uniform mat4 model1;
uniform mat4 model2;
uniform mat4 view;
uniform mat4 projection;
uniform int useMatrix; // NOTE: 0 for quaternion, 1 for matrix
void main() {
    if (useMatrix == 0) {
        gl_Position = projection * view * model1 * vec4(position, 1.0);
    } else {
        gl_Position = projection * view * model2 * vec4(position, 1.0);
    }
}
)";

// NOTE: Fragment Shader source code
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 color;
void main() {
    color = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

// NOTE: Cube vertices
GLfloat vertices[] = {
        // NOTE: Front face
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        // NOTE: Back face
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        // NOTE: Top face
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        // NOTE: Bottom face
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        // NOTE: Right face
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        // NOTE: Left face
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f
};

// NOTE: Cube indices
GLuint indices[] = {
        0, 1, 2, 2, 3, 0,       // NOTE: Front face
        4, 5, 6, 6, 7, 4,       // NOTE: Back face
        8, 9, 10, 10, 11, 8,    // NOTE: Top face
        12, 13, 14, 14, 15, 12, // NOTE: Bottom face
        16, 17, 18, 18, 19, 16, // NOTE: Right face
        20, 21, 22, 22, 23, 20  // NOTE: Left face
};

GLuint createShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
    }

    return shader;
}

GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    return program;
}

void applyRotationWithQuaternion(const Quaternion& q, GLfloat* matrix) {
    QuaternionMatrix rotationMatrix = q.toRotationMatrix();
    GLfloat tempMatrix[16] = {
            rotationMatrix.a1, rotationMatrix.a2, rotationMatrix.a3, rotationMatrix.a4,
            rotationMatrix.b1, rotationMatrix.b2, rotationMatrix.b3, rotationMatrix.b4,
            rotationMatrix.c1, rotationMatrix.c2, rotationMatrix.c3, rotationMatrix.c4,
            rotationMatrix.d1, rotationMatrix.d2, rotationMatrix.d3, rotationMatrix.d4
    };
    for (int i = 0; i < 16; ++i) {
        matrix[i] = tempMatrix[i];
    }
}

void applyRotationWithMatrix(float angle, GLfloat* matrix) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    matrix[0] = cosA; matrix[4] = -sinA; matrix[8] = 0.0f; matrix[12] = 0.0f;
    matrix[1] = sinA; matrix[5] = cosA;  matrix[9] = 0.0f; matrix[13] = 0.0f;
    matrix[2] = 0.0f; matrix[6] = 0.0f;  matrix[10] = 1.0f; matrix[14] = 0.0f;
    matrix[3] = 0.0f; matrix[7] = 0.0f;  matrix[11] = 0.0f; matrix[15] = 1.0f;
}

void applyTranslation(GLfloat x, GLfloat y, GLfloat z, GLfloat* matrix) {
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
}

int main() {
    // NOTE: Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // NOTE: Setup GLFW window properties
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // NOTE: Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Math - Quaternion with OpenGL - ESGI", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return -1;
    }

    // NOTE: Make the window's context current
    glfwMakeContextCurrent(window);

    // NOTE: Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // NOTE: Build and compile shaders
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = createProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // NOTE: Setup cube VAO and VBO
    GLuint VAO[2], VBO[2], EBO[2];
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(2, EBO);

    for (int i = 0; i < 2; ++i) {
        glBindVertexArray(VAO[i]);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // NOTE: Define basic quaternions for rotations
    Quaternion q1 = Quaternion(cos(M_PI / 8), sin(M_PI / 8), 0, 0); // NOTE: 45-degree rotation around x-axis
    Quaternion q2 = Quaternion(cos(M_PI / 8), 0, sin(M_PI / 8), 0); // NOTE: 45-degree rotation around y-axis

    GLfloat matrix1[16], matrix2[16];

    // NOTE: Enable depth test
    glEnable(GL_DEPTH_TEST);

    // NOTE: Ensure we can capture keys being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // NOTE: Loop until the user closes the window or press esc
    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        // NOTE: Calculate the angle of rotation based on time
        float timeValue = (float)glfwGetTime();
        float angle = timeValue * M_PI / 4; // NOTE: Rotate 45 degrees per second
        Quaternion q_rotation = Quaternion(cos(angle / 2), sin(angle / 2) * 1, sin(angle / 2) * 1, 0).getUnit();

        // NOTE: Compose rotations
        Quaternion q_composed = q1.mutliply(q_rotation).mutliply(q2);

        // NOTE: Apply rotations
        applyRotationWithQuaternion(q_composed, matrix1);
        applyRotationWithMatrix(angle, matrix2);

        // NOTE: Apply translations
        applyTranslation(-2.0f, 0.0f, 0.0f, matrix1); // NOTE: Move left cube (quaternion) to the left
        applyTranslation(2.0f, 0.0f, 0.0f, matrix2);  // NOTE: Move right cube (matrix) to the right

        // NOTE: Clear the colorbuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // NOTE: Use the shader program
        glUseProgram(shaderProgram);

        // NOTE: Get matrix's uniform location and set matrix
        GLuint modelLoc1 = glGetUniformLocation(shaderProgram, "model1");
        GLuint modelLoc2 = glGetUniformLocation(shaderProgram, "model2");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        GLuint useMatrixLoc = glGetUniformLocation(shaderProgram, "useMatrix");

        // NOTE: Pass them to the shaders
        glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, matrix1);
        glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, matrix2);

        // NOTE: Camera/View transformation
        GLfloat view[16] = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, -5, 1
        };
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);

        // NOTE: Projection
        GLfloat projection[16] = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, -1, -1,
                0, 0, -2, 0
        };
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);

        // NOTE: Draw the cube using quaternion rotations
        glUniform1i(useMatrixLoc, 0);
        glBindVertexArray(VAO[0]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // NOTE: Draw the cube using matrix rotations
        glUniform1i(useMatrixLoc, 1);
        glBindVertexArray(VAO[1]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        // NOTE: Swap the screen buffers
        glfwSwapBuffers(window);

        // NOTE: Poll for and process events
        glfwPollEvents();
    }

    // NOTE: Properly de-allocate all resources once they've outlived their purpose
    for (int i = 0; i < 2; ++i) {
        glDeleteVertexArrays(1, &VAO[i]);
        glDeleteBuffers(1, &VBO[i]);
        glDeleteBuffers(1, &EBO[i]);
    }

    // NOTE: Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}
