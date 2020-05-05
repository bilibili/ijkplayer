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


typedef struct IjkDemoInfo {
    IjkFFMediaPlayer *fp;
    int state;
    int w, h;
    int isSeeking;
    int64_t duration;
} IjkDemoInfo;

void demo_event_cb(void *userdata, int what, int arg1, int arg2, void *extra) {
    printf("demo event cb what %5d:(%5d %5d)\n", what, arg1, arg2);

    IjkDemoInfo *info = userdata;
    switch (what) {
        case IJK_MSG_PLAYBACK_STATE_CHANGED:
            info->state = arg1;
            break;
        case IJK_MSG_PREPARED:
            info->duration = ijkff_get_duration(info->fp);
            break;
        case IJK_MSG_VIDEO_SIZE_CHANGED:
            info->w = arg1;
            info->h = arg2;
            break;
        case IJK_MSG_SEEK_COMPLETE:
            info->isSeeking = 0;
            break;
        default:
            break;
    }
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

int processEvent(IjkFFMediaPlayer *fp, IjkDemoInfo *info, const SDL_Event *event) {
    if (!fp || !event)
        return SDL_QUIT;

    int ret = 0;
    switch (event->type) {
        case SDL_QUIT:
            ret = SDL_QUIT;
            break;
        case SDL_KEYDOWN: {
            switch (event->key.keysym.sym) {
                case SDLK_SPACE: {
                    if (info->state == IJK_STATE_STARTED)
                        ijkff_pause(fp);
                    else if (info->state == IJK_STATE_PAUSED)
                        ijkff_start(fp);
                    break;
                }
                case SDLK_UP:
                case SDLK_DOWN: {
                    float volume = ijkff_get_playback_volume(fp);
                    volume += event->key.keysym.sym == SDLK_DOWN ? -0.1f : 0.1f;
                    if (volume < 0.0f) volume = 0.0f;
                    if (volume > 1.0f) volume = 1.0f;
                    ijkff_set_playback_volume(fp, volume);
                    break;
                }
                case SDLK_LEFT:
                case SDLK_RIGHT: {
                    if (ijkff_is_playing(fp)) {
                        int64_t position = ijkff_get_current_position(fp);
                        int64_t delta = info->duration / 40;
                        int64_t target = position + (event->key.keysym.sym == SDLK_LEFT ? -delta : delta);
                        if (target < 0) target = 0;
                        if (target > info->duration) target = info->duration;
                        ijkff_seek_to(fp, target);
                    }
                    break;
                }
                case SDLK_ESCAPE:
                    ret = SDL_QUIT;
                    break;
                default:
                    break;
            }
            default:
                break;
        }
    }
    return ret;
}

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

    IjkFFMediaPlayer *fp = ijkff_create(IJK_VOUT_SDL2);

    IjkDemoInfo info;
    memset(&info, 0, sizeof(info));
    info.fp = fp;
    ijkff_set_event_cb(fp, &info, demo_event_cb);

    ijkff_set_option(fp, "fcc-bgra", "overlay-format", IJK_OPT_CATEGORY_PLAYER);
    ijkff_set_int_option(fp, 1, "start-on-prepared", IJK_OPT_CATEGORY_PLAYER);
    ijkff_set_data_source(fp, "https://player.alicdn.com/video/aliyunmedia.mp4");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);


    SDL_Window *window = SDL_CreateWindow("Video",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          640, 480,
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    ijkff_set_window(fp, window);
    ijkff_prepare_async(fp);
    //ijkff_start(fp);


#if GLFW
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glfwPollEvents();
    }
#endif

    SDL_Event event;
    while (1) {
        SDL_WaitEvent(&event);
        if (processEvent(fp, &info, &event) == SDL_QUIT)
            break;
    }
    SDL_Quit();

    ijkff_stop(fp);
    ijkff_shutdown(fp);

    return 0;
}
