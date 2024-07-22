#define TINYOBJLOADER_IMPLEMENTATION

#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <filesystem>
#include "library.h"
#include "tiny_obj_loader.h"

const GLfloat FOVY = 90;
const GLint WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
const GLfloat NEAR = .01f, FAR = 1000;
const GLfloat MOUSE_SENSITIVITY = .001f;

// NOTE: Vertex Shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
out vec3 fragColor;
uniform mat4 model;
void main() {
    gl_Position = model * vec4(position, 1.0);
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

float cameraPitch = 0;
float cameraYaw = 0;

struct Vertex {
    GLfloat position[3];
    GLfloat color[3];
    GLfloat normal[3];
    GLfloat texcoords[2];
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

std::vector<Vertex> applyRotationWithQuaternion(Quaternion& q, std::vector<Vertex> vertices, Double3 origin = Double3(0, 0, 0)) {
    std::vector<Vertex> newVector;
    for (int i = 0; i < vertices.size(); ++i)
    {
        Double3 vertex(vertices[i].position[0], vertices[i].position[1], vertices[i].position[2]);
        Double3 rotatedVertex = vertex.rotate(q, origin);
        Vertex newVertex;
        newVertex.position[0] = rotatedVertex.x;
        newVertex.position[1] = rotatedVertex.z;
        newVertex.position[2] = rotatedVertex.y;
        newVertex.color[0] = vertices[i].color[0];
        newVertex.color[1] = vertices[i].color[1];
        newVertex.color[2] = vertices[i].color[2];
        newVector.push_back(newVertex);
    }

    return newVector;
}

void applyRotationWithMatrix(Quaternion& q, GLfloat* matrix) {
    RotationMatrix quaternionMatrix = q.getRotationMatrix();
    matrix[0] = quaternionMatrix.a1; matrix[1] = quaternionMatrix.a2; matrix[2] = quaternionMatrix.a3;
    matrix[4] = quaternionMatrix.b1; matrix[5] = quaternionMatrix.b2; matrix[6] = quaternionMatrix.b3;
    matrix[8] = quaternionMatrix.c1; matrix[9] = quaternionMatrix.c2; matrix[10] = quaternionMatrix.c3;
    matrix[15] = 1;
}

void applyTranslation(GLfloat x, GLfloat y, GLfloat z, QuaternionMatrix* matrix) {
    matrix->d1 += x;
    matrix->d2 += y;
    matrix->d3 += z;
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

    cameraYaw -= xDiff * MOUSE_SENSITIVITY;
    cameraPitch += yDiff * MOUSE_SENSITIVITY;
}

void processShape(tinyobj::shape_t shape, tinyobj::attrib_t attrib, std::vector<tinyobj::material_t> materials) {

    int index_offset = 0;
    for (int f = 0; f < shape.mesh.num_face_vertices.size(); ++f)
    {
        int fv = shape.mesh.num_face_vertices[f];
        int mid = shape.mesh.material_ids[f];

        for (int v = 0; v < fv; ++v)
        {
            Vertex vertex;

            tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
            // SET VERTEX POSITION
            vertex.position[0] = attrib.vertices[3 * idx.vertex_index + 0];
            vertex.position[1] = attrib.vertices[3 * idx.vertex_index + 1];
            vertex.position[2] = attrib.vertices[3 * idx.vertex_index + 2];

            // SET VERTEX COLOR
            vertex.color[0] = materials[mid].diffuse[0];
            vertex.color[1] = materials[mid].diffuse[1];
            vertex.color[2] = materials[mid].diffuse[2];

            modelIndices.push_back(modelVertices.size());
            modelVertices.push_back(vertex);
        }

        index_offset += fv;
    }

//    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
//        aiFace face = mesh->mFaces[i];
//        for (unsigned int j = 0; j < face.mNumIndices; j++) {
//            modelIndices.push_back(face.mIndices[j]);
//        }
//    }
}

void loadModel(const std::string& path) {
    tinyobj::ObjReaderConfig config;
    tinyobj::ObjReader importer;

    if (!importer.ParseFromFile(path, config))
    {
        std::cerr << "TinyObjReader: " << importer.Error() << std::endl;
        exit(1);
    }

    auto& attrib = importer.GetAttrib();
    auto& shapes = importer.GetShapes();
    auto& materials = importer.GetMaterials();

    for (int i = 0; i < shapes.size(); ++i)
        processShape(shapes[i], attrib, materials);
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
    std::filesystem::path modelPath = std::filesystem::current_path() / "landscape.obj";
    std::cout << "Attempting to load model from path: " << modelPath << std::endl;

    // NOTE: Load the model
    loadModel(modelPath.string());

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

    QuaternionMatrix modelMatrix = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            2.0f, -1.0f, 0.0f, 1
    };

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

        Quaternion q_rotationCamera = Quaternion::eulerAngles(cameraYaw, Double3(0, 1, 0)).multiply(Quaternion::eulerAngles(cameraPitch, Double3(1, 0, 0)));
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
            cameraTranslation = cameraTranslation.rotate(q_rotationCamera);
            cameraTranslation = cameraTranslation.add(centerPosition);
        } else {
            Double3 forwardVector = Double3(0, 0, 1).rotate(q_composedCamera);
            Double3 rightVector = Double3(1, 0, 0).rotate(q_composedCamera);
            Double3 upVector = Double3(0, -1, 0).rotate(q_composedCamera);

            // Movement
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.subtract(forwardVector.multiply(deltaTime));
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.add(forwardVector.multiply(deltaTime));
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.subtract(rightVector.multiply(deltaTime));
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.add(rightVector.multiply(deltaTime));
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.subtract(upVector.multiply(deltaTime));
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                cameraTranslation = cameraTranslation.add(upVector.multiply(deltaTime));
        }

        // Clamp vertical rotation between -90° and 90°
        cameraPitch = fmin(fmax(-M_PI / 2, cameraPitch), M_PI / 2);
        // NOTE: Apply rotations
        // Quaternion cubeAnimationRotation = Quaternion::eulerAngles(angle, {0, 1, 1});
        // applyRotationWithQuaternion(cubeAnimationRotation, vertices, sizeof(vertices) / sizeof(vertices[0]));

        // Update the vertices of the tree model
        glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        glBufferData(GL_ARRAY_BUFFER, modelVertices.size() * sizeof(Vertex), &modelVertices[0], GL_STATIC_DRAW);

        // NOTE: Clear the colorbuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // NOTE: Use the shader program
        glUseProgram(shaderProgram);

        // NOTE: Get matrix's uniform location and set matrix
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");

        RotationMatrix rotationMatrix = q_composedCamera.getRotationMatrix();

        // Inverse/Transposed rotationMatrix
        QuaternionMatrix viewRotation = {
            rotationMatrix.a1, rotationMatrix.b1, rotationMatrix.c1, 0,
            rotationMatrix.a2, rotationMatrix.b2, rotationMatrix.c2, 0,
            rotationMatrix.a3, rotationMatrix.b3, rotationMatrix.c3, 0,
            0, 0, 0, 1
        };

        // Inverse cameraTranslation
        // Irregularities on d2 and d3 due to origin change between our calculations and OpenGL!
        // DO NOT CHANGE!
        QuaternionMatrix viewTranslation = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            -cameraTranslation.x, cameraTranslation.z, -cameraTranslation.y, 1
        };

        // NOTE: Camera/View transformation
        QuaternionMatrix view = viewTranslation.multiply(viewRotation);

        // NOTE: Projection
        float aspect = WINDOW_WIDTH / WINDOW_HEIGHT;
        float f = 1 / tan(FOVY / 2);
        QuaternionMatrix projection = {
            f / aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (FAR + NEAR) / (NEAR - FAR), -1,
            0, 0, 2 * NEAR * FAR / (NEAR - FAR), 0
        };

        QuaternionMatrix finalMatrix = modelMatrix.multiply(view.multiply(projection));

        GLfloat finalMatrixTab[16]
        {
            (float)finalMatrix.a1, (float)finalMatrix.a2, (float)finalMatrix.a3, (float)finalMatrix.a4,
            (float)finalMatrix.b1, (float)finalMatrix.b2, (float)finalMatrix.b3, (float)finalMatrix.b4,
            (float)finalMatrix.c1, (float)finalMatrix.c2, (float)finalMatrix.c3, (float)finalMatrix.c4,
            (float)finalMatrix.d1, (float)finalMatrix.d2, (float)finalMatrix.d3, (float)finalMatrix.d4
        };

        // NOTE: Pass them to the shaders
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, finalMatrixTab);


        // NOTE: Draw the model
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