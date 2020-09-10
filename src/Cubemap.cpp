#include "Cubemap.h"
#include "Shader.h"
#include "ComputeShader.h"
#include "glad/glad.h"
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

//void renderCube();

/*glm::mat4 Cubemap::captureViews[] =
{
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

glm::mat4 Cubemap::captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);*/

/* ORDER */
/*
    "right.jpg",
    "left.jpg",
    "top.jpg",
    "bottom.jpg",
    "front.jpg",
    "back.jpg"
*/

// Parameters
static constexpr int kEnvMapSize = 1024;
static constexpr int kIrradianceMapSize = 32;
static constexpr int kBRDF_LUT_Size = 256;

void Cubemap::CreateFromFiles(const std::vector<std::string>& files)
{
    stbi_set_flip_vertically_on_load(0);

    glGenTextures(1, &m_gl_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_id);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < files.size(); i++)
    {
        unsigned char* data = stbi_load(files[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            std::cout << "Cubemap texture loaded successfully: " << files[i] << std::endl;
        }
        else
            std::cout << "Cubemap texture failed to load at path: " << files[i] << std::endl;
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
/*
    "_0.jpg",
    "_1.jpg",
    "_2.jpg",
    "_3.jpg",
    "_4.jpg",
    "_5.jpg"
*/
void Cubemap::CreateFromFiles(const std::string& name, const std::string& extension)
{
    std::vector<std::string> files;
    files.resize(6);
    for (int i = 0; i < 6; i++)
        files[i] = name + "_" + std::to_string(i) + extension;

    CreateFromFiles(files);
}

void Cubemap::CreateFomHDR(const HdrTexture& hdrTexture)
{
    ComputeShader cs;
    cs.CreateFromFile("assets/shaders/equirect2cube_cs.shader");

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_gl_id);
    glTextureStorage2D(m_gl_id, 1, GL_RGB16F, kEnvMapSize, kEnvMapSize);
    glTextureParameteri(m_gl_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_gl_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    cs.Bind();
    glBindTextureUnit(0, hdrTexture.m_gl_id);
    glBindImageTexture(0, m_gl_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glDispatchCompute(kEnvMapSize / 32, kEnvMapSize / 32, 6);
}
/*
void Cubemap::CreateIrradiance(const Cubemap& other)
{
    // pbr: setup framebuffer
    // ----------------------
    unsigned int captureFBO;
    unsigned int captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    glGenTextures(1, &m_gl_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_id);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
    // -----------------------------------------------------------------------------
    Shader irradianceShader;
    irradianceShader.CreateFromFiles("res/shaders/cubemapV.shader", "res/shaders/irradianceConvolutionF.shader");
    irradianceShader.SetUniform1i("environmentMap", 0);
    irradianceShader.SetUniformMatrix4fv("projection", &(captureProjection[0][0]));
    irradianceShader.Bind();

    other.Bind(0);

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader.SetUniformMatrix4fv("view", &(captureViews[i][0][0]));
        //irradianceShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_gl_id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
*/
Cubemap::~Cubemap()
{
    glDeleteTextures(1, &m_gl_id);
}

void Cubemap::Bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_id);
}

void Cubemap::Unbind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
