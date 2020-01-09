#include <stdio.h>
#include <SDL.h>
#include <glad/glad.h>
#include <tgl/context.hpp>
#include <tgl/render_target.hpp>
#include <tgl/blending.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <tgl/mesh.hpp>
#include <tgl/shader.hpp>
#include <tl/colors.hpp>
#include <tl/random.hpp>
#include <cgltf.h>

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

uchar buffer[4*1024*1024];

cgltf_data* loadMesh(const char* fileName)
{
    FILE* file = fopen(fileName, "rb");
    if(file == nullptr)
        return nullptr;
    fseek(file, 0, SEEK_END);
    const size_t length = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(buffer, 1, length, file);
    fclose(file);
    cgltf_options options = {};
    options.type = cgltf_file_type_invalid;
    cgltf_data* parsedData = nullptr;
    cgltf_result result = cgltf_parse(&options, buffer, length, &parsedData);
    if(result == cgltf_result_success) {
        cgltf_load_buffers(&options, parsedData, fileName);
        return parsedData;
    }
    else {
        fprintf(stderr, "error loading gltf mesh\n");
    }
    return nullptr;
}

static glm::mat4 meshRotation(1);
static bool mousePressed = false;
static float cameraDist = 2.f;
void processMouse(float dt, int dx, int dy)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    meshRotation = glm::eulerAngleXY(3.14f * dy / h, 3.14f * dx / w) * meshRotation;
}

void launch_test_2()
{
    tl::randSeed();
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    window = SDL_CreateWindow("title",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    // SDL_GL_SetSwapInterval(1); // Enable vsync


    if(gladLoadGL() == 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    }
    tgl::enableDepthTest(true);
    tgl::blending::enable();
    tgl::enableMultisampling();
    tgl::viewport(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);
    tgl::scissor(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);

    auto meshData = loadMesh("monkey.glb");
    auto& mesh = meshData->meshes[0];

    auto ebo = tgl::Ebo::create();
    auto vboPos = tgl::VboT<glm::vec3>::create();
    auto vboColor = tgl::VboT<glm::vec3>::create();
    auto vao = tgl::Vao::create();

    const u32 numVerts = mesh.primitives->attributes[0].data->count;
    const u32 numInds = mesh.primitives->indices->count;

    auto posBufferView = mesh.primitives->attributes[0].data->buffer_view;
    vboPos.uploadData((const char*)posBufferView->buffer->data, posBufferView->size);
    tl::Vector<glm::vec3> vColors(numVerts);
    for(u32 i = 0; i < numVerts; i += 3) {
        const float r = 0.001f * tl::randI32(1000);
        const float g = 0.001f * tl::randI32(1000);
        const float b = 0.001f * tl::randI32(1000);
        vColors[i+0] = {r, g, b};
        vColors[i+1] = {r, g, b};
        vColors[i+2] = {r, g, b};
    }
    vboColor.upload(vColors);
    auto indsBuffer = (const char*)mesh.primitives[0].indices->buffer_view->buffer->data;
    auto indsOffset = mesh.primitives[0].indices->buffer_view->offset;
    ebo.uploadData(indsBuffer + indsOffset, sizeof(u16)*numInds);

    vao.link(0, vboPos.attribRef<0>());
    vao.link(1, vboColor.attribRef<0>());
    vao.link(ebo);

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
            case SDL_MOUSEBUTTONDOWN:
                mousePressed = true;
                break;
            case SDL_MOUSEBUTTONUP:
                mousePressed = false;
                break;
            case SDL_MOUSEMOTION:
                if(mousePressed)
                    processMouse(dt, event.motion.xrel, event.motion.yrel);
                break;
            case SDL_MOUSEWHEEL:
                cameraDist -= 0.1f*event.wheel.y;
                cameraDist = tl::max(0.1f, cameraDist);
                break;
            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    const i32 w = event.window.data1;
                    const i32 h = event.window.data2;
                    tgl::viewport(0, 0, w, h);
                    tgl::scissor(0, 0, w, h);
                }
                break;
            }
        }

        shader.use();
        tgl::clear({0, 0.1f, 0.1f, 0.f});
        int windowW, windowH;
        SDL_GetWindowSize(window, &windowW, &windowH);
        const auto modelMtx = meshRotation;
        const auto projMtx = glm::perspective(glm::radians(60.f), (float)windowW / windowH, 0.1f, 100.f);
        const auto viewMtx = glm::translate(glm::mat4(1), {0, 0, -cameraDist});
        const auto modelViewProj = projMtx * viewMtx * modelMtx;
        shader.set("u_modelViewProj", modelViewProj);
        tgl::drawElements(vao, tgl::Primitive::TRIANGLES, numInds, tgl::Ebo::Type::U16);

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
