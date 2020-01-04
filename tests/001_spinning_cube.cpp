#include <stdio.h>
#include <SDL.h>
#include <glad/glad.h>
#include <tgl/context.hpp>
#include <tgl/render_target.hpp>
#include <tgl/blending.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <tgl/mesh.hpp>
#include <tgl/shader.hpp>
#include <tl/colors.hpp>
#include <tl/random.hpp>

static SDL_Window* window = nullptr;
static SDL_GLContext glContext;
static bool gameLoopRunning = true;

static const char vertShaderSrc[] = R"GLSL(
#version 330

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;

out vec3 color;

uniform mat4 u_modelViewProj;

void main()
{
    color = a_color;
    gl_Position = u_modelViewProj * vec4(a_pos, 1.0);
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

constexpr int numVerts = 36;
static const glm::vec3 trianglePositions[numVerts] = {
    // FRONT
    {-0.5f, -0.5f, +0.5f}, {+0.5f, -0.5f, +0.5f}, {+0.5f, +0.5f, +0.5f},
    {-0.5f, -0.5f, +0.5f}, {+0.5f, +0.5f, +0.5f}, {-0.5f, +0.5f, +0.5f},
    // BACK
    {-0.5f, -0.5f, -0.5f}, {+0.5f, +0.5f, -0.5f}, {+0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f, -0.5f}, {-0.5f, +0.5f, -0.5f}, {+0.5f, +0.5f, -0.5f},
    // LEFT
    {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, +0.5f}, {-0.5f, +0.5f, +0.5f}, 
    {-0.5f, -0.5f, -0.5f}, {-0.5f, +0.5f, +0.5f}, {-0.5f, +0.5f, -0.5f},
    // RIGHT
    {+0.5f, -0.5f, -0.5f}, {+0.5f, +0.5f, +0.5f}, {+0.5f, -0.5f, +0.5f},
    {+0.5f, -0.5f, -0.5f}, {+0.5f, +0.5f, -0.5f}, {+0.5f, +0.5f, +0.5f},
    // TOP
    {-0.5f, +0.5f, +0.5f}, {+0.5f, +0.5f, +0.5f}, {+0.5f, +0.5f, -0.5f},
    {-0.5f, +0.5f, +0.5f}, {+0.5f, +0.5f, -0.5f}, {-0.5f, +0.5f, -0.5f},
    // DOWN
    {-0.5f, -0.5f, +0.5f}, {+0.5f, -0.5f, -0.5f}, {+0.5f, -0.5f, +0.5f},
    {-0.5f, -0.5f, +0.5f}, {-0.5f, -0.5f, -0.5f}, {+0.5f, -0.5f, -0.5f},
};
static const glm::vec3 triangleColors[numVerts] = {
    // FRONT
    tl::colors::red(), tl::colors::red(), tl::colors::red(),
    tl::colors::red(), tl::colors::red(), tl::colors::red(),
    // BACK
    tl::colors::green(), tl::colors::green(), tl::colors::green(),
    tl::colors::green(), tl::colors::green(), tl::colors::green(),
    // LEFT
    tl::colors::blue(), tl::colors::blue(), tl::colors::blue(),
    tl::colors::blue(), tl::colors::blue(), tl::colors::blue(),
    // RIGHT
    tl::colors::yellow(), tl::colors::yellow(), tl::colors::yellow(),
    tl::colors::yellow(), tl::colors::yellow(), tl::colors::yellow(),
    // TOP
    tl::colors::cyan(), tl::colors::cyan(), tl::colors::cyan(),
    tl::colors::cyan(), tl::colors::cyan(), tl::colors::cyan(),
    // DOWN
    tl::colors::magenta(), tl::colors::magenta(), tl::colors::magenta(),
    tl::colors::magenta(), tl::colors::magenta(), tl::colors::magenta(),
};

constexpr int INIT_WINDOW_WIDTH = 800;
constexpr int INIT_WINDOW_HEIGHT = 600;

void launch_test_1()
{
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

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
    tgl::enableDepthTest(true);
    tgl::blending::enable();
    tgl::enableMultisampling();
    tgl::viewport(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);
    tgl::scissor(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);

    auto vboPos = tgl::VboT<glm::vec3>::create();
    auto vboColor = tgl::VboT<glm::vec3>::create();
    auto vao = tgl::Vao::create();
    vboPos.upload(trianglePositions);
    vboColor.upload(triangleColors);
    vao.link(0, vboPos.attribRef<0>());
    vao.link(1, vboColor.attribRef<0>());

    auto randSpeed = []{ return 0.001f * tl::randI32(-1000, +1000); };
    glm::vec3 cubeRot = {0,0,0};
    glm::vec3 prevRotationSpeed = {randSpeed(), randSpeed(), 0};
    glm::vec3 nextRotationSpeed = {randSpeed(), randSpeed(), 0};
    const float timeChangeRotSpeeds = 2.5f;
    float timeSinceChangeRotSpeeds = 0.f;

    auto shader = tgl::ShaderProgram::create(vertShaderSrc, fragShaderSrc);

    int prevTicks = SDL_GetTicks();
    while(gameLoopRunning)
    {
        const int newTicks = SDL_GetTicks();
        const int deltaTicks = newTicks - prevTicks;
        const float t = 0.001f * newTicks;
        const float dt = 0.001f * deltaTicks;
        prevTicks = newTicks;
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
        tgl::clear({0, 0.1f, 0.1f, 0.f});
        int windowW, windowH;
        SDL_GetWindowSize(window, &windowW, &windowH);
        if(timeSinceChangeRotSpeeds > timeChangeRotSpeeds) {
            timeSinceChangeRotSpeeds = 0.f;
            prevRotationSpeed = nextRotationSpeed;
            nextRotationSpeed = {randSpeed(), randSpeed(), 0};
        }
        glm::vec3 rotSpeed = glm::lerp(prevRotationSpeed, nextRotationSpeed, timeSinceChangeRotSpeeds / timeChangeRotSpeeds);
        cubeRot += rotSpeed * dt;
        const auto modelMtx = glm::yawPitchRoll(cubeRot.x, cubeRot.y, cubeRot.z);
        const auto projMtx = glm::perspective(glm::radians(60.f), (float)windowW / windowH, 0.1f, 100.f);
        const auto viewMtx = glm::translate(glm::mat4(1), {0, 0, -2});
        const auto modelViewProj = projMtx * viewMtx * modelMtx;
        shader.set("u_modelViewProj", modelViewProj);
        tgl::draw(vao, tgl::Primitive::TRIANGLES, numVerts);

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
