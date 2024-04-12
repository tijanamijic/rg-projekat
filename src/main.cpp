#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <rg/Error.h>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadTexture(const char *path);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(3.0f, 3.0f, 15.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

glm::vec3 windowPosition = glm::vec3(0.0f, 25.0f, 0.0f);
float windowScale = 15.0f;


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 planePosition = glm::vec3(0.2f,5.0f,0.2f);
float planeScale = 0.7f;


struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};


int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);


    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    float transparentVertices[] = {
            // positions         //normals         // texture Coords
            25.0f,  0.0f,  25.0f, 0.0f, 1.0f, 0.0f, 40.0f, 0.0f,
            -25.0f,  0.0f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 40.0f,
            -25.0f,  0.0f,  25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

            25.0f,  0.0f,  25.0f, 0.0f, 1.0f, 0.0f, 40.0f, 0.0f,
            25.0f,  0.0f, -25.0f, 0.0f, 1.0f, 0.0f, 40.0f, 40.0f,
            -25.0f,  0.0f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 40.0f
    };

    float transparentVertices2[] = {
            // positions         // texture Coords
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader planeShader("resources/shaders/plane.vs", "resources/shaders/plane.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader waterShader("resources/shaders/water.vs", "resources/shaders/water.fs");
    Shader thunderShader("resources/shaders/thunder.vs","resources/shaders/thunder.fs");



    unsigned int thunderTex = loadTexture(FileSystem::getPath("resources/textures/blend.png").c_str());
    // transparent thunder locations
    // --------------------------------
    vector<glm::vec3> thunder
            {
                    glm::vec3(3.2f,3.0f,3.2f)
            };

    unsigned int transparentVAO2, transparentVBO2;
    glGenVertexArrays(1, &transparentVAO2);
    glGenBuffers(1, &transparentVBO2);
    glBindVertexArray(transparentVAO2);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices2), transparentVertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);



    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    float skyboxVertices[] = {
            // positions
            -50.0f,  50.0f, -50.0f,
            -50.0f, -50.0f, -50.0f,
            50.0f, -50.0f, -50.0f,
            50.0f, -50.0f, -50.0f,
            50.0f,  50.0f, -50.0f,
            -50.0f,  50.0f, -50.0f,

            -50.0f, -50.0f,  50.0f,
            -50.0f, -50.0f, -50.0f,
            -50.0f,  50.0f, -50.0f,
            -50.0f,  50.0f, -50.0f,
            -50.0f,  50.0f,  50.0f,
            -50.0f, -50.0f,  50.0f,

            50.0f, -50.0f, -50.0f,
            50.0f, -50.0f,  50.0f,
            50.0f,  50.0f,  50.0f,
            50.0f,  50.0f,  50.0f,
            50.0f,  50.0f, -50.0f,
            50.0f, -50.0f, -50.0f,

            -50.0f, -50.0f,  50.0f,
            -50.0f,  50.0f,  50.0f,
            50.0f,  50.0f,  50.0f,
            50.0f,  50.0f,  50.0f,
            50.0f, -50.0f,  50.0f,
            -50.0f, -50.0f,  50.0f,

            -50.0f,  50.0f, -50.0f,
            50.0f,  50.0f, -50.0f,
            50.0f,  50.0f,  50.0f,
            50.0f,  50.0f,  50.0f,
            -50.0f,  50.0f,  50.0f,
            -50.0f,  50.0f, -50.0f,

            -50.0f, -50.0f, -50.0f,
            -50.0f, -50.0f,  50.0f,
            50.0f, -50.0f, -50.0f,
            50.0f, -50.0f, -50.0f,
            -50.0f, -50.0f,  50.0f,
            50.0f, -50.0f,  50.0f
    };

    // load models
    // -----------
    Model planeModel("resources/objects/caravanazip/source/42cc9aa7d22b4afa917d6f84e6c5642f/caravanA.obj");
    planeModel.SetShaderTextureNamePrefix("material.");


    thunderShader.use();
    thunderShader.setInt("texture1", 0);
    //for (auto& textures : planeModel.textures_loaded) {
      //  LOG(std::cerr) << textures.path << ' ' << textures.type << '\n';
    //}

    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/water.png").c_str());
    vector<glm::vec3> waterSquares
            {
                    glm::vec3(-25.0f, 1.0f, -25.0f),
                    glm::vec3(-25.0f, 1.0f, 25.0f),
                    glm::vec3(25.0f, 1.0f, -25.0f),
                    glm::vec3(25.0f, 1.0f, 25.0f)
            };

    // skybox VAO,VBO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load textures
    // -------------
    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox/right.jpg"),
                    FileSystem::getPath("resources/textures/skybox/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox/back.jpg"),
            };
    unsigned int cubemapTexture = loadCubemap(faces);

    // shader configuration
    // --------------------
    planeShader.use();
    planeShader.setInt("skybox", 0);


    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    PointLight pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(1.0, 1.0, 1.0);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;


    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        waterShader.setBool("celShading", true);


        // render
        // ------
        glClearColor(0.0f,0.0f,0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        planeShader.use();
        pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        planeShader.setVec3("pointLight.position", pointLight.position);
        planeShader.setVec3("pointLight.ambient", pointLight.ambient);
        planeShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        planeShader.setVec3("pointLight.specular", pointLight.specular);
        planeShader.setFloat("pointLight.constant", pointLight.constant);
        planeShader.setFloat("pointLight.linear", pointLight.linear);
        planeShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        planeShader.setVec3("viewPosition", camera.Position);
        planeShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations



        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,planePosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(planeScale));

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        planeShader.setMat4("model", model);
        planeShader.setMat4("view", view);
        planeShader.setMat4("projection", projection);
        planeShader.setVec3("cameraPos", camera.Position);
        planeModel.Draw(planeShader);


        std::map<float, glm::vec3> sorted;
        for (auto & waterSquare : waterSquares)
        {
            float distance = glm::length(camera.Position - waterSquare);
            sorted[distance] = waterSquare;
        }

        waterShader.use();
        waterShader.setVec3("viewPos", camera.Position);

        waterShader.setMat4("projection", projection);
        waterShader.setMat4("view", view);
        waterShader.setFloat("currentFrame", currentFrame);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glBindVertexArray(transparentVAO);
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            waterShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }


        thunderShader.use();
        //blending
        glBindVertexArray(transparentVAO2);
        glBindTexture(GL_TEXTURE_2D, thunderTex);
        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            thunderShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        //skybox
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}


unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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