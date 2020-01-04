#include <stdio.h>
#include <SDL.h>
#include <glad/glad.h>
#include <tgl/context.hpp>
#include <tgl/render_target.hpp>
#include <glm/vec3.hpp>
#include <tgl/mesh.hpp>
#include <tgl/shader.hpp>

static SDL_Window* window = nullptr;
static SDL_GLContext glContext;
static bool gameLoopRunning = true;

static const char vertShaderSrc[] = R"GLSL(
#version 330

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;

out vec3 color;

void main()
{
    color = a_color;
    gl_Position = vec4(a_pos, 1.0);
}
)GLSL";

static const char fragShaderSrc[] = R"GLSL(
#version 330

layout(location = 0) out vec4 o_color;

in vec3 color;

void main()
{
    o_color = vec4(color, 1.0);
}
)GLSL";

static const glm::vec3 trianglePositions[3] = {
    {-0.5f, -0.5f, 0}, {+0.5f, -0.5f, 0}, {0, +0.5f, 0}
};
static const glm::vec3 triangleColors[3] = {
    {1, 0, 0}, {0, 1, 0}, {0, 0, 1}
};

constexpr int INIT_WINDOW_WIDTH = 800;
constexpr int INIT_WINDOW_HEIGHT = 600;

void launch_test_0()
{
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    window = SDL_CreateWindow("title",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    if(gladLoadGL() == 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    }
    tgl::viewport(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);
    tgl::scissor(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);

    auto vboPos = tgl::VboT<glm::vec3>::create();
    auto vboColor = tgl::VboT<glm::vec3>::create();
    auto vao = tgl::Vao::create();
    vboPos.upload(trianglePositions);
    vboColor.upload(triangleColors);
    vao.link(0, vboPos.attribRef<0>());
    vao.link(1, vboColor.attribRef<0>());

    auto shader = tgl::ShaderProgram::create(vertShaderSrc, fragShaderSrc);

    while(gameLoopRunning)
    {
        static SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_QUIT:
                gameLoopRunning = false;
                break;
            }
        }

        shader.use();
        tgl::clearColor(0, 0.1f, 0.1f, 0.f);
        tgl::draw(vao, tgl::Primitive::TRIANGLES, 3);

        SDL_GL_SwapWindow(window);
    }

    shader.free();
    vboPos.free();
    vboColor.free();
    vao.free();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
