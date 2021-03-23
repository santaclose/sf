#include "IblTools.h"
#include "Config.h"

#include <glad/glad.h>
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image_write.h>
#include <iostream>

namespace IblTools {
    
    unsigned int envToCubeFBO, envToCubeRBO, irradianceFBO, irradianceRBO, prefilterFBO, prefilterRBO, brdfLUTFBO, brdfLUTRBO;
    Shader equirectangularToCubemapShader, irradianceShader, prefilterShader, integrateShader;

    glm::mat4 envMapProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 envMapView[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };


    unsigned int renderCubeVAO = 0, renderCubeVBO;
    unsigned int renderQuadVAO = 0, renderQuadVBO;
    float cubeVertices[] =
    {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
        // right face
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
         1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left      
    };
    //float quadVertices[] =
    //{
    //  // positions       // texture Coords
    //    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    //    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    //     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
    //     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    //};
    float quadVertices[] =
    {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };
    void renderCube()
    {
        if (renderCubeVAO == 0)
        {
            glGenVertexArrays(1, &renderCubeVAO);
            glGenBuffers(1, &renderCubeVBO);
            // fill buffer
            glBindBuffer(GL_ARRAY_BUFFER, renderCubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
            // link vertex attributes
            glBindVertexArray(renderCubeVAO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        glBindVertexArray(renderCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
    void renderQuad()
    {
        if (renderQuadVAO == 0)
        {
            glGenVertexArrays(1, &renderQuadVAO);
            glGenBuffers(1, &renderQuadVBO);

            glBindBuffer(GL_ARRAY_BUFFER, renderQuadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

            glBindVertexArray(renderQuadVAO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        glBindVertexArray(renderQuadVAO);
        //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
}

void IblTools::HdrToCubemaps(const Texture& hdrTexture, Cubemap& environmentCubemap, Cubemap& irradianceCubemap, Cubemap& prefilterCubemap, Texture& lookupTexture)
{
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    equirectangularToCubemapShader.CreateFromFiles("assets/shaders/ibl/equirectangularToCubemapV.shader", "assets/shaders/ibl/equirectangularToCubemapF.shader");
    irradianceShader.CreateFromFiles("assets/shaders/ibl/irradianceV.shader", "assets/shaders/ibl/irradianceF.shader");
    prefilterShader.CreateFromFiles("assets/shaders/ibl/prefilterV.shader", "assets/shaders/ibl/prefilterF.shader");
    integrateShader.CreateFromFiles("assets/shaders/ibl/integrateV.shader", "assets/shaders/ibl/integrateF.shader");

    // Latlong to Cubemap conversion
    glGenFramebuffers(1, &envToCubeFBO);
    glGenRenderbuffers(1, &envToCubeRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, envToCubeFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, envToCubeRBO);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, environmentCubemap.GetSize(), environmentCubemap.GetSize());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, envToCubeRBO);

    equirectangularToCubemapShader.Bind();
    equirectangularToCubemapShader.SetUniform1i("envMap", 0);
    equirectangularToCubemapShader.SetUniformMatrix4fv("projection", glm::value_ptr(envMapProjection));
    hdrTexture.Bind();
    environmentCubemap.Bind();

    glViewport(0, 0, environmentCubemap.GetSize() , environmentCubemap.GetSize());
    glBindFramebuffer(GL_FRAMEBUFFER, envToCubeFBO);

    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader.SetUniformMatrix4fv("view", glm::value_ptr(envMapView[i]));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environmentCubemap.GlId(), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();

        /*float* debugBuffer = new float[environmentCubemap.GetSize() * environmentCubemap.GetSize() * 3];

        glReadPixels(0, 0, environmentCubemap.GetSize(), environmentCubemap.GetSize(), GL_RGB, GL_FLOAT, debugBuffer);
        int asdf = stbi_write_hdr(("debug_" + std::to_string(i) + ".hdr").c_str(), environmentCubemap.GetSize(), environmentCubemap.GetSize(), 3, debugBuffer);
        delete[] debugBuffer;
        if (!asdf)
        {
            std::cout << "STB WRITE ERROR\n";
        }*/
    }

    environmentCubemap.ComputeMipmap();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Diffuse irradiance capture
    glGenFramebuffers(1, &irradianceFBO);
    glGenRenderbuffers(1, &irradianceRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, irradianceFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, irradianceRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceCubemap.GetSize(), irradianceCubemap.GetSize());

    irradianceShader.Bind();

    irradianceShader.SetUniformMatrix4fv("projection", glm::value_ptr(envMapProjection));
    glActiveTexture(GL_TEXTURE0);
    environmentCubemap.Bind();

    glViewport(0, 0, irradianceCubemap.GetSize(), irradianceCubemap.GetSize());
    glBindFramebuffer(GL_FRAMEBUFFER, irradianceFBO);

    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader.SetUniformMatrix4fv("view", glm::value_ptr(envMapView[i]));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceCubemap.GlId(), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();

        /*float* debugBuffer = new float[irradianceCubemap.GetSize() * irradianceCubemap.GetSize() * 3];
        glReadPixels(0, 0, irradianceCubemap.GetSize(), irradianceCubemap.GetSize(), GL_RGB, GL_FLOAT, debugBuffer);
        int asdf = stbi_write_hdr(("debug_" + std::to_string(i) + ".hdr").c_str(), irradianceCubemap.GetSize(), irradianceCubemap.GetSize(), 3, debugBuffer);
        delete[] debugBuffer;
        if (!asdf)
        {
            std::cout << "STB WRITE ERROR\n";
        }*/
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Prefilter cubemap
    prefilterShader.Bind();

    prefilterShader.SetUniformMatrix4fv("projection", glm::value_ptr(envMapProjection));
    environmentCubemap.Bind();

    glGenFramebuffers(1, &prefilterFBO);
    glGenRenderbuffers(1, &prefilterRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, prefilterFBO);

    unsigned int maxMipLevels = 5;

    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        unsigned int mipWidth = prefilterCubemap.GetSize() * std::pow(0.5, mip);
        unsigned int mipHeight = prefilterCubemap.GetSize() * std::pow(0.5, mip);

        glBindRenderbuffer(GL_RENDERBUFFER, prefilterRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);

        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);

        prefilterShader.SetUniform1f("roughness", roughness);
        prefilterShader.SetUniform1f("cubeResolutionWidth", prefilterCubemap.GetSize());
        prefilterShader.SetUniform1f("cubeResolutionHeight", prefilterCubemap.GetSize());

        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader.SetUniformMatrix4fv("view", glm::value_ptr(envMapView[i]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterCubemap.GlId(), mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderCube();

            /*float* debugBuffer = new float[prefilterCubemap.GetSize() * prefilterCubemap.GetSize() * 3];
            glReadPixels(0, 0, prefilterCubemap.GetSize(), prefilterCubemap.GetSize(), GL_RGB, GL_FLOAT, debugBuffer);
            int asdf = stbi_write_hdr(("debug_" + std::to_string(i) + ".hdr").c_str(), prefilterCubemap.GetSize(), prefilterCubemap.GetSize(), 3, debugBuffer);
            delete[] debugBuffer;
            if (!asdf)
            {
                std::cout << "STB WRITE ERROR\n";
            }*/
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // BRDF LUT
    glDisable(GL_DEPTH_TEST);
    lookupTexture.Bind();
    glGenFramebuffers(1, &brdfLUTFBO);
    glGenRenderbuffers(1, &brdfLUTRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, brdfLUTFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, brdfLUTRBO);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, lookupTexture.GetWidth(), lookupTexture.GetHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lookupTexture.GlId(), 0);

    glViewport(0, 0, lookupTexture.GetWidth(), lookupTexture.GetHeight());
    integrateShader.Bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderQuad();

    /*float* debugBuffer = new float[lookupTexture.GetWidth() * lookupTexture.GetHeight() * 3];
    glReadPixels(0, 0, lookupTexture.GetWidth(), lookupTexture.GetHeight(), GL_RGB, GL_FLOAT, debugBuffer);
    int asdf = stbi_write_hdr("debug.hdr", lookupTexture.GetWidth(), lookupTexture.GetHeight(), 3, debugBuffer);
    delete[] debugBuffer;
    if (!asdf)
    {
        std::cout << "STB WRITE ERROR\n";
    }*/

    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, Config::windowWidth, Config::windowHeight);

    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
}