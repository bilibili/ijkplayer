/*****************************************************************************
* ijkplayer_demo.c
*****************************************************************************
*
* copyright (c) 2019 befovy <befovy@gmail.com>
*
* This file is part of ijkPlayer.
*
* ijkPlayer is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* ijkPlayer is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with ijkPlayer; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "ijkplayer_desktop.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <SDL.h>

#define SCR_WIDTH   800
#define SCR_HEIGHT  600

void demo_event_cb(void *userdata, int what, int arg1, int arg2, void *extra) {
     printf("demo event cb what %5d:(%5d %5d)\n", what, arg1, arg2);
}

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}


//void processInput(GLFWwindow *window) {
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//}

//static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
//    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, GLFW_TRUE);
//}

int main(int argc, char *argv[]) {
#if GLFW
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if APPLE
//     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
    if (window == NULL) {
        //cout << "Failed to create window" << endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowPos(window, 200, 200);
    glfwShowWindow(window);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    // gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwMakeContextCurrent(NULL);
#endif

    IjkFFMediaPlayer *fp = ijkff_create();
    ijkff_set_option(fp, "fcc-i420", "overlay-format", IJK_OPT_CATEGORY_PLAYER);
    ijkff_set_data_source(fp, "http://player.alicdn.com/video/aliyunmedia.mp4");
    // ijkff_set_data_source(fp, "/Users/bytedance/Downloads/aliyunmedia.mp4");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);

    SDL_Window *window = SDL_CreateWindow("Video",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          640, 480,
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    ijkff_set_window(fp, window);
    ijkff_prepare_async(fp);
    ijkff_start(fp);

    ijkff_set_event_cb(fp, NULL, demo_event_cb);

#if GLFW
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glfwPollEvents();
    }
#endif

    SDL_Event event;
    while (1) {
        SDL_WaitEvent(&event);

        if (event.type == SDL_QUIT)
            break;
    }
    SDL_Quit();


    ijkff_stop(fp);
    ijkff_shutdown(fp);

//    glfwTerminate();
    return 0;
}
