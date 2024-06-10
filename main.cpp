#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>
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

void applyRotationWithQuaternion(Quaternion& q, GLfloat* vertices, int vertexCount, Double3 origin = Double3(0, 0, 0)) {
    for (int i = 0; i < vertexCount; i += 3) {
        Double3 vertex(vertices[i], vertices[i + 1], vertices[i + 2]);
        Double3 rotatedVertex = vertex.rotate(q, origin);
        vertices[i] = rotatedVertex.x;
        vertices[i + 1] = rotatedVertex.z;
        vertices[i + 2] = rotatedVertex.y;
    }
}

void applyRotationWithMatrix(Quaternion& q, GLfloat* matrix) {
    RotationMatrix quaternionMatrix = q.getRotationMatrix();
    matrix[0] = quaternionMatrix.a1; matrix[1] = quaternionMatrix.a2; matrix[2] = quaternionMatrix.a3;
    matrix[4] = quaternionMatrix.b1; matrix[5] = quaternionMatrix.b2; matrix[6] = quaternionMatrix.b3;
    matrix[8] = quaternionMatrix.c1; matrix[9] = quaternionMatrix.c2; matrix[10] = quaternionMatrix.c3;
    matrix[15] = 1;
}

void applyTranslation(GLfloat x, GLfloat y, GLfloat z, GLfloat* matrix) {
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
}

void printMatrix(const GLfloat* matrix, const char* name) {
    printf("%s:\n", name);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            printf("%f ", matrix[i * 4 + j]);
        }
        printf("\n");
    }
}

void printVertices(const GLfloat* vertices, int vertexCount) {
    printf("Vertices:\n");
    for (int i = 0; i < vertexCount; i += 3) {
        printf("%f %f %f\n", vertices[i], vertices[i + 1], vertices[i + 2]);
    }
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
    Quaternion q1 = Quaternion::eulerAngles(0, Double3(1, 0, 0));
    Quaternion q2 = Quaternion::eulerAngles(0, Double3(1, 0, 0));

    GLfloat matrix1[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
    };

    GLfloat matrix2[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
    };

    // NOTE: Copy of original vertices for the left cube
    GLfloat originalVertices[sizeof(vertices) / sizeof(vertices[0])];
    memcpy(originalVertices, vertices, sizeof(vertices));

    // NOTE: Enable depth test
    glEnable(GL_DEPTH_TEST);

    // NOTE: Ensure we can capture keys being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    Double3 cameraTranslation = Double3(0, 0, 0);
    float cameraPitch = 0;
    float cameraYaw = 0;

    // NOTE: Loop until the user closes the window or press esc
    float timeValue;
    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        // NOTE: Calculate the angle of rotation based on time
        float previousTime = timeValue;
        timeValue = (float)glfwGetTime();
        float deltaTime = timeValue - previousTime;
        float angle = timeValue * M_PI / 4; // NOTE: Rotate 45 degrees per second

        // Camera Controls
        // Movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraTranslation.z += 1 * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraTranslation.z -= 1 * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraTranslation.x += 1 * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraTranslation.x -= 1 * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            cameraTranslation.y += 1 * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraTranslation.y -= 1 * deltaTime;

        // Rotation
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            cameraPitch -= M_PI * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            cameraPitch += M_PI * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
            cameraYaw += M_PI * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            cameraYaw -= M_PI * deltaTime;

        Quaternion q_rotation = Quaternion::eulerAngles(cameraPitch, Double3(1, 0, 0)).multiply(Quaternion::eulerAngles(cameraYaw, Double3(0, 1, 0)));

        // NOTE: Compose rotations
        Quaternion q_composed = q1.multiply(q_rotation).multiply(q2).getUnit();

        // NOTE: Reset vertices to original before applying the rotation
        memcpy(vertices, originalVertices, sizeof(vertices));

        // NOTE: Apply rotations
        applyRotationWithQuaternion(q_composed, vertices, sizeof(vertices) / sizeof(vertices[0]), Double3(2 + cameraTranslation.x, cameraTranslation.y, cameraTranslation.z));
        printf("%lf %lf %lf\n", cameraTranslation.x, cameraTranslation.y, cameraTranslation.z);
//        applyRotationWithMatrix(q_composed, matrix2);

        // NOTE: Apply translations
        applyTranslation(-2.0f, 0.0f, .0f, matrix1); // NOTE: Move left cube (quaternion) to the left
//        applyTranslation(cameraTranslation.x + 2.0f, cameraTranslation.y + 0.0f, cameraTranslation.z + .0f, matrix2);  // NOTE: Move right cube (matrix) to the right

        // NOTE: Print matrices and vertices for debugging
        // printMatrix(matrix1, "Matrix1 (Cube Left)");
        // printMatrix(matrix2, "Matrix2 (Cube Right)");
        // printVertices(vertices, sizeof(vertices) / sizeof(vertices[0]));

        // NOTE: Update the vertices of the left cube
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
//        glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, matrix2);

        // NOTE: Camera/View transformation
        GLfloat view[16] = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                (GLfloat)cameraTranslation.x, (GLfloat)cameraTranslation.y, (GLfloat)cameraTranslation.z, 1
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
