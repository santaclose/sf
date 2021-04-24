#include "Skybox.h"

#include <Components/Camera.h>

bool sf::Skybox::generated = false;
unsigned int sf::Skybox::gl_VAO;
unsigned int sf::Skybox::gl_VBO;

float sf::Skybox::cubeVertices[] = {   
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};
sf::Shader sf::Skybox::shader;
sf::Cubemap* sf::Skybox::cubemap;

void sf::Skybox::SetCubemap(Cubemap* cubemap)
{
    if (!generated)
    {
        glGenVertexArrays(1, &gl_VAO);
        glGenBuffers(1, &gl_VBO);

        glBindVertexArray(gl_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, gl_VBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

        glBindVertexArray(0);
        generated = true;
    }

    if (cubemap->IsHDR())
        shader.CreateFromFiles("assets/shaders/skyboxV.shader", "assets/shaders/skyboxHdrF.shader");
    else
        shader.CreateFromFiles("assets/shaders/skyboxV.shader", "assets/shaders/skyboxF.shader");

    sf::Skybox::cubemap = cubemap;
}

void sf::Skybox::SetUseExposure(bool value)
{
    shader.Bind();
    shader.SetUniform1i("useExposure", (int)value);
}

void sf::Skybox::SetExposure(float value)
{
    shader.Bind();
    shader.SetUniform1f("exposure", value);
}


void sf::Skybox::Draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!generated)
        return;

    glDisable(GL_DEPTH_TEST);

    shader.Bind();

    glm::mat4 fixedViewMat = glm::mat4(glm::mat3(viewMatrix));
    shader.SetUniformMatrix4fv("view", &(fixedViewMat[0][0]));
    shader.SetUniformMatrix4fv("projection", &(projectionMatrix[0][0]));
    glBindVertexArray(gl_VAO);
    cubemap->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}