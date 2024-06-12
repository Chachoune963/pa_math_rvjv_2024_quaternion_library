#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "library.h"

const GLint WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
const GLfloat MOUSE_SENSITIVITY = .001f;

// NOTE: Vertex Shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
out vec3 fragColor;
uniform mat4 model1;
uniform mat4 model2;
uniform mat4 model3;
uniform mat4 view;
uniform mat4 projection;
uniform int useMatrix; // NOTE: 0 for quaternion, 1 for matrix
void main() {
    if (useMatrix == 0) {
        gl_Position = projection * view * model1 * vec4(position, 1.0);
    } else if (useMatrix == 1) {
        gl_Position = projection * view * model2 * vec4(position, 1.0);
    } else {
        gl_Position = projection * view * model3 * vec4(position, 1.0);
    }
    fragColor = color;
}
)";

// NOTE: Fragment Shader source code
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 fragColor;
out vec4 color;
void main() {
    color = vec4(fragColor, 1.0);
}
)";

// NOTE: Cube vertices
GLfloat vertices[] = {
        // The first three: Positions         The last three: Colours
        // NOTE: Front face
        -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        // NOTE: Back face
        -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        // NOTE: Top face
        -1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        // NOTE: Bottom face
        -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        // NOTE: Right face
        1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        // NOTE: Left face
        -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f
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

float cameraPitch = 0;
float cameraYaw = 0;

struct Vertex {
    GLfloat position[3];
    GLfloat color[3];
};

std::vector<Vertex> modelVertices;
std::vector<GLuint> modelIndices;

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
    for (int i = 0; i < vertexCount; i += 6) {
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

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    double xOrigin = WINDOW_WIDTH / 2;
    double yOrigin = WINDOW_HEIGHT / 2;

    double xDiff = xpos - xOrigin;
    double yDiff = ypos - yOrigin;

    cameraYaw += xDiff * MOUSE_SENSITIVITY;
    cameraPitch += yDiff * MOUSE_SENSITIVITY;
}

void processMesh(aiMesh* mesh, const aiScene* scene) {
    aiColor4D color;
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color);
    } else {
        color = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f); // Default to white if no material
    }

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position[0] = mesh->mVertices[i].x;
        vertex.position[1] = mesh->mVertices[i].y;
        vertex.position[2] = mesh->mVertices[i].z;

        vertex.color[0] = color.r;
        vertex.color[1] = color.g;
        vertex.color[2] = color.b;

        modelVertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            modelIndices.push_back(face.mIndices[j]);
        }
    }
}

void processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

void loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    processNode(scene->mRootNode, scene);
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

    // NOTE: Define the model path
    std::string modelPath = "/Users/michaelattal/Developments/esgi/projet_annuel/3eme_annee/pa_math_rvjv_2024_quaternion_library/tree.fbx";
    std::cout << "Attempting to load model from path: " << modelPath << std::endl;

    // NOTE: Load the model
    loadModel(modelPath);

    // NOTE: Build and compile shaders
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = createProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // NOTE: Setup cube VAO and VBO
    GLuint VAO[3], VBO[3], EBO[3];
    glGenVertexArrays(3, VAO);
    glGenBuffers(3, VBO);
    glGenBuffers(3, EBO);

    // Setup Cube
    for (int i = 0; i < 2; ++i) {
        glBindVertexArray(VAO[i]);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // Setup Model
    glBindVertexArray(VAO[2]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, modelVertices.size() * sizeof(Vertex), &modelVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelIndices.size() * sizeof(GLuint), &modelIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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

    GLfloat modelMatrix[16] = {
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    Double3 cameraTranslation = Double3(0, 0, 0);

    bool centeredCamera = false;
    Double3 centerPosition = Double3(0, 0, 0);
    float centeredOffset = 5;

    // NOTE: Loop until the user closes the window or press esc
    float timeValue;
    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        glfwSetCursorPos(window, WINDOW_WIDTH / 2,  WINDOW_HEIGHT / 2);

        // NOTE: Calculate the angle of rotation based on time
        float previousTime = timeValue;
        timeValue = (float)glfwGetTime();
        float deltaTime = timeValue - previousTime;
        float angle = timeValue * M_PI / 4; // NOTE: Rotate 45 degrees per second

        Quaternion q_rotation = Quaternion::eulerAngles(cameraPitch, Double3(1, 0, 0)).multiply(Quaternion::eulerAngles(cameraYaw, Double3(0, 1, 0)));
        // NOTE: Compose rotations
        Quaternion q_composed = q_rotation.getUnit();

        // Camera Controls
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            centeredCamera = true;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            centeredCamera = false;

        // Rotation
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            cameraPitch -= M_PI * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            cameraPitch += M_PI * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
            cameraYaw += M_PI * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            cameraYaw -= M_PI * deltaTime;

        Quaternion q_rotationCamera = Quaternion::eulerAngles(cameraYaw, Double3(0, 0, 1)).multiply(Quaternion::eulerAngles(cameraPitch, Double3(1, 0, 0)));
        // NOTE: Compose rotations
        Quaternion q_composedCamera = q_rotationCamera.getUnit();

        if (centeredCamera)
        {
            // Centered Movement
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                centeredOffset -= 1 * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                centeredOffset += 1 * deltaTime;

            cameraTranslation = Double3(0, -centeredOffset, 0);
            cameraTranslation = cameraTranslation.rotate(q_composedCamera);
            cameraTranslation = cameraTranslation.add(centerPosition);
        } else {
            Double3 forwardVector = Double3(0, 1, 0).rotate(q_composedCamera);
            Double3 rightVector = Double3(1, 0, 0).rotate(q_composedCamera);
            Double3 upVector = Double3(0, 0, 1).rotate(q_composedCamera);

            // Movement
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.add(forwardVector.multiply(deltaTime));
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.subtract(forwardVector.multiply(deltaTime));
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.add(rightVector.multiply(deltaTime));
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.subtract(rightVector.multiply(deltaTime));
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.add(upVector.multiply(deltaTime));
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.subtract(upVector.multiply(deltaTime));
        }

        // Clamp vertical rotation between -90° and 90°
        cameraPitch = fmin(fmax(-M_PI / 2, cameraPitch), M_PI / 2);

        // NOTE: Reset vertices to original before applying the rotation
        memcpy(vertices, originalVertices, sizeof(vertices));

        // NOTE: Apply rotations
        applyRotationWithQuaternion(q_composed, vertices, sizeof(vertices) / sizeof(vertices[0]), Double3(-cameraTranslation.x, 5 - cameraTranslation.z, -cameraTranslation.y));

        // NOTE: Apply translations
        applyTranslation(0.0f, 0.0f, -5.0f, matrix1);
        applyTranslation(cameraTranslation.x + 2.0f, cameraTranslation.y + 0.0f, cameraTranslation.z + 0.0f, modelMatrix);

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
        GLuint modelLoc3 = glGetUniformLocation(shaderProgram, "model3");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        GLuint useMatrixLoc = glGetUniformLocation(shaderProgram, "useMatrix");

        // NOTE: Pass them to the shaders
        glUniformMatrix4fv(modelLoc1, 1, GL_FALSE, matrix1);
        glUniformMatrix4fv(modelLoc3, 1, GL_FALSE, modelMatrix);

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

        // NOTE: Draw the model
        glUniform1i(useMatrixLoc, 2);
        glBindVertexArray(VAO[2]);
        glDrawElements(GL_TRIANGLES, modelIndices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        // NOTE: Swap the screen buffers
        glfwSwapBuffers(window);

        // NOTE: Poll for and process events
        glfwPollEvents();
    }

    // NOTE: Properly de-allocate all resources once they've outlived their purpose
    for (int i = 0; i < 3; ++i) {
        glDeleteVertexArrays(1, &VAO[i]);
        glDeleteBuffers(1, &VBO[i]);
        glDeleteBuffers(1, &EBO[i]);
    }

    // NOTE: Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}