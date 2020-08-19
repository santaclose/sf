#include "Skybox.h"

bool Skybox::generated = false;
unsigned int Skybox::gl_VAO;
unsigned int Skybox::gl_VBO;

float Skybox::cubeVertices[] = {   
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
Shader Skybox::shader;
Cubemap* Skybox::cubemap;

void Skybox::Generate(Cubemap* cubemap)
{
    glGenVertexArrays(1, &gl_VAO);
    glGenBuffers(1, &gl_VBO);

    glBindVertexArray(gl_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, gl_VBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glBindVertexArray(0);

    shader.CreateFromFiles("assets/shaders/skyboxV.shader", "assets/shaders/skyboxF.shader");
    generated = true;

    Skybox::cubemap = cubemap;
}

void Skybox::Draw()
{
    if (!generated)
        return;

    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

    shader.Bind();

    glm::mat4 viewMatrix = glm::mat4(glm::mat3(Camera::GetViewMatrix()));
    shader.SetUniformMatrix4fv("view", &(viewMatrix[0][0]));
    shader.SetUniformMatrix4fv("projection", &(Camera::GetProjectionMatrix()[0][0]));
    glBindVertexArray(gl_VAO);
    cubemap->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);  // change depth function so depth test passes when values are equal to depth buffer's content
}
