//========================================================================
// OpenGL 3.3 triangle example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#include "getopt.h"

typedef struct Vertex
{
    vec3 pos;
    vec3 col;
} Vertex;

static const Vertex vertices[6] =
{
    { { -1.0f, -1.0f, 0.0f }, { 1.f, 0.f, 0.f } },
    { {  1.0f, -1.0f, 0.0f }, { 0.f, 1.f, 0.f } },
    { { -1.0f,  1.0f, 0.0f }, { 0.f, 0.f, 1.f } },

    { { -1.0f,  1.0f, 0.0f }, { 0.f, 1.f, 0.f } },
    { {  1.0f, -1.0f, 0.0f }, { 0.f, 0.f, 1.f } },
    { {  1.0f,  1.0f, 0.0f }, { 1.f, 0.f, 0.f } },
};

static const char* vertex_shader_text =
"#version 330\n"
"precision mediump float;\n"
"uniform int CellX;\n"
"uniform int CellY;\n"
"uniform int Layers;\n"
"attribute vec3 vCol;\n"
"attribute vec3 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    int layer_size = CellX * CellY;\n"
"    int layer_idx = gl_InstanceID / layer_size;\n"
"    int layer_remain = gl_InstanceID % layer_size;\n"
"    int y_idx = layer_remain / CellX;\n"
"    int x_idx = layer_remain % CellX;\n"
"    float step_x = 2.0 / float(CellX);\n"
"    float step_y = 2.0 / float(CellY);\n"
"    float step_z = 1.0 / float(Layers);\n"
"    gl_Position = vec4(step_x * float(x_idx) - 1.0 + (vPos.x + 1.0) * step_x * 0.5, step_y * float(y_idx) - 1.0 + (vPos.y + 1.0) * step_y * 0.5, 1.0 - step_z * layer_idx, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"precision mediump float;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

static void usage(void)
{
    printf("Usage: triangle-test [OPTION]...\n");
    printf("Options:\n");
    printf("  -w, --width=WIDTH        the window width in pixels, default: 1024\n");
    printf("  -g, --height=HEIGHT      the window height in pixels, default: 768\n");
    printf("  -x, --cells-in-x=X       the number of grid cells in horizontal, default: 16\n");
    printf("  -y, --cells-in-y=Y       the number of grid cells in vertical, default: 12\n");
    printf("  -z, --layers=Z           the number of layers, default: 4\n");
    printf("      --enable-depth-test  enable depth test\n");
    printf("      --enable-cull-face   enable cull face\n");
    printf("      --polygon-mode=MODE  polygon mode must be: 'point', 'line' or 'fill', default: 'fill'\n");
    printf("  -f, --frames=FRAMES      number of rendering frames, default: 100\n");
    printf("  -h, --help               show this help\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void parse_int(int* n, char* name)
{
    int t = atoi(optarg);
    if (t > 0)
    {
        *n = t;
    }
    else
    {
        fprintf(stderr, "`%s` must be a number greater than zero!\n", name);
        exit(EXIT_FAILURE);
    }
}

void checkCompileErrors(unsigned int shader, int type) /* type: 0 = shader; 1 = program */
{
    int success;
    char infoLog[1024];
    if (type != 1)
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            printf("ERROR::SHADER_COMPILATION_ERROR of type: %d\n%s\n", type, infoLog);
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            printf("ERROR::PROGRAM_LINKING_ERROR of type: %d\n%s\n", type, infoLog);

        }
    }
}

const char* polygon_mode_name(GLenum polygon_mode)
{
    assert(polygon_mode == GL_POINT || polygon_mode == GL_LINE || polygon_mode == GL_FILL);
    if (polygon_mode == GL_POINT)
    {
        return "GL_POINT";
    }
    else if (polygon_mode == GL_LINE)
    {
        return "GL_LINE";
    }
    else
    {
        return "GL_FILL";
    }
}

int main(int argc, char** argv)
{
    int width = 1024; // window width in pixels
    int height = 768; // window height in pixels
    int x = 16;       // number of grid cells in horizontal
    int y = 12;       // number of grid cells in vertical
    int z = 4;        // number of layers
    int enable_depth_test = 0;
    int enable_cull_face = 0;
    GLenum polygon_mode = GL_FILL; // point, line, fill
    int frames = 100;

    int ch;

    enum {
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        CELLS_IN_X,
        CELLS_IN_Y,
        LAYERS,
        ENABLE_DEPTH_TEST,
        ENABLE_CULL_FACE,
        POLYGON_MODE,
        FRAMES,
        HELP
    };

    const struct option options[] =
    {
        { "width",              1, NULL, WINDOW_WIDTH },
        { "height",             1, NULL, WINDOW_HEIGHT },
        { "cells-in-x",         1, NULL, CELLS_IN_X },
        { "cells-in-y",         1, NULL, CELLS_IN_Y },
        { "layers",             1, NULL, LAYERS },
        { "enable-depth-test",  0, NULL, ENABLE_DEPTH_TEST },
        { "enable-cull-face",   0, NULL, ENABLE_CULL_FACE },
        { "polygon-mode",       1, NULL, POLYGON_MODE },
        { "frames",             1, NULL, FRAMES },
        { "help",               0, NULL, HELP },
        { NULL, 0, NULL, 0 }
    };

    while ((ch = getopt_long(argc, argv, "w:g:x:y:z:f:h", options, NULL)) != -1)
    {
        switch (ch)
        {
        case 'w':
        case WINDOW_WIDTH:
            if (strcmp(optarg, "-") != 0)
            {
                parse_int(&width, "width");
            }
            break;
        case 'g':
        case WINDOW_HEIGHT:
            if (strcmp(optarg, "-") != 0)
            {
                parse_int(&height, "height");
            }
            break;
        case 'x':
        case CELLS_IN_X:
            if (strcmp(optarg, "-") != 0)
            {
                parse_int(&x, "cells-in-x");
            }
            break;
        case 'y':
        case CELLS_IN_Y:
            if (strcmp(optarg, "-") != 0)
            {
                parse_int(&y, "cells-in-y");
            }
            break;
        case 'z':
        case LAYERS:
            if (strcmp(optarg, "-") != 0)
            {
                parse_int(&z, "layers");
            }
            break;
        case ENABLE_DEPTH_TEST:
            enable_depth_test = 1;
            break;
        case ENABLE_CULL_FACE:
            enable_cull_face = 1;
            break;
        break;
        case POLYGON_MODE:
            if (strcmp(optarg, "point") == 0)
            {
                polygon_mode = GL_POINT;
            }
            else if (strcmp(optarg, "line") == 0)
            {
                polygon_mode = GL_LINE;
            }
            else if (strcmp(optarg, "fill") == 0)
            {
                polygon_mode = GL_FILL;
            }
            else
            {
                fprintf(stderr, "'--polygon-mode' must be 'point', 'line' or 'fill'!\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'f':
        case FRAMES:
            if (strcmp(optarg, "-") != 0)
            {
                parse_int(&frames, "frames");
            }
            break;
        case 'h':
        case HELP:
            usage();
            exit(EXIT_SUCCESS);
            break;
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }

    printf("======== parameters ========\n");
    printf("window width: %d\n", width);
    printf("window height: %d\n", height);
    printf("cells in x: %d\n", x);
    printf("cells in y: %d\n", y);
    printf("layers: %d\n", z);
    printf("frames: %d\n", frames);
    printf("enable depth test: %s\n", enable_depth_test ? "yes" : "no");
    printf("enable cull face: %s\n", enable_cull_face ? "yes" : "no");
    printf("polygon mode: %s\n", polygon_mode_name(polygon_mode));


    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL 3.3 Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    checkCompileErrors(vertex_shader, 0);

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
    checkCompileErrors(fragment_shader, 0);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    checkCompileErrors(program, 1);

    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");

    const GLint cellx_location = glGetUniformLocation(program, "CellX");
    const GLint celly_location = glGetUniformLocation(program, "CellY");
    const GLint layer_location = glGetUniformLocation(program, "Layers");


    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glEnableVertexAttribArray(vpos_location);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, col));

    if (enable_depth_test)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    if (enable_cull_face)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
    glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);

    GLuint qry;
    glGenQueries(1, &qry);

    GLuint64 ns = 0;
    GLuint64 total_ns = 0;
    GLfloat ms = 0; // 1 ns = 10^(-6) ms

    int fc = 0;

    GLbitfield clear_mask = GL_COLOR_BUFFER_BIT;
    if (enable_depth_test)
    {
        clear_mask |= GL_DEPTH_BUFFER_BIT;
    }

    while (!glfwWindowShouldClose(window) && (fc < frames))
    {
        glViewport(0, 0, width, height);

        glClear(clear_mask);

        glUseProgram(program);
        glUniform1i(cellx_location, x);
        glUniform1i(celly_location, y);
        glUniform1i(layer_location, z);

        glBeginQuery(GL_TIME_ELAPSED, qry);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, x*y*z);
        glEndQuery(GL_TIME_ELAPSED);
        
        glFinish();

        int isAvailable = GL_FALSE;
        glGetQueryObjectiv(qry,
            GL_QUERY_RESULT_AVAILABLE,
            &isAvailable);
        if (!isAvailable)
        {
            fprintf(stderr, "GL_QUERY_RESULT_AVAILABLE!\n");
            break;
        }

        glGetQueryObjectui64v(qry, GL_QUERY_RESULT, &ns);
        total_ns += ns;

        glfwSwapBuffers(window);
        glfwPollEvents();

        ++fc;

        {
            static char title[128];
            sprintf(title, "OpenGL 3.3 Triangle - Frame %d", fc);
            glfwSetWindowTitle(window, title);
        }
    }

    printf("======== results ========\n");
    printf("grid: %d x %d\n", x, y);
    printf("layers: %d\n", z);
    int triangles = 2 * x * y * z;
    printf("triangles per frame: %d\n", triangles);
    printf("frames: %d\n", frames);
    printf("total time: %fs\n", total_ns / 1000000000.0);
    printf("average frame time: %fs\n", total_ns / 1000000000.0 / frames);
    printf("triangles per second (TPS): %f\n", triangles / (total_ns / 1000000000.0 / frames));

    glDeleteQueries(1, &qry);

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

