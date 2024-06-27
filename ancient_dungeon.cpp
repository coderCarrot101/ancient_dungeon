#include <iostream>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "./constants.h"
#include <bits/stdc++.h>
#include <cmath>
using namespace std;

const int MAP_WIDTH = 50; 
const int MAP_HEIGHT = 50; 
const int TILE_SIZE = 16;

int game_is_running {false};
int last_frame_time {0};
int input_update[20];
SDL_Window *window = NULL;
int flag = 0;
SDL_Renderer *renderer = NULL;
char path[50];
char * pPath;
char cwd[50];

int game_map[MAP_WIDTH][MAP_HEIGHT];

//TODO:
/*
> make a raycaster for the player's FOV
*/

struct game_object {
    float x;
    float y;
    float width;
    float height;
    float vel_x;
    float vel_y;
    float direction;
    float radDirection = (direction / 180 ) * 3.1415926535;
    float tempMovX = 0;
    float tempMovY = 0;
} ball;

int initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }
    window = SDL_CreateWindow(
        "Ancient Dungeon",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    if (!window) {
        fprintf(stderr, "Error creating SDL Window.\n");
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL Renderer.\n");
        return false;
    }
    return true;
}

int * process_input(int * input_update) {

    SDL_Event event;
    static bool keys[SDL_NUM_SCANCODES] = { false };  // Initialize all keys to false
    int * pInput_update = input_update;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                game_is_running = false;
                break;
            case SDL_KEYDOWN:
                keys[event.key.keysym.scancode] = true;
                break;
            case SDL_KEYUP:
                keys[event.key.keysym.scancode] = false;
                break;
        }
    }

    if (keys[SDL_SCANCODE_W]) {
        *pInput_update = 'w';
        pInput_update++;
    } 
    if (keys[SDL_SCANCODE_A]) {
        *pInput_update = 'a';
        pInput_update++;
    }
    if (keys[SDL_SCANCODE_S]) {
        *pInput_update = 's';
        pInput_update++;
    }
    if (keys[SDL_SCANCODE_D]) {
        *pInput_update = 'd';
        pInput_update++;
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
        *pInput_update = 'r';
        pInput_update++;
    }
    if (keys[SDL_SCANCODE_LEFT]) {
        *pInput_update = 'l';
        pInput_update++;
    }

    *pInput_update = '0';
    return input_update;
}

void load_map() {
    fstream file;
    string word;
    file.open("mapA.map");
    int x = 0, y = 0;

    while (file >> word) {
        if (word == "01,") {
            game_map[x / TILE_SIZE][y / TILE_SIZE] = 1;
        }
        x += TILE_SIZE;
        if (x >= MAP_WIDTH * TILE_SIZE) {
            x = 0;
            y += TILE_SIZE;
        }
    }
}

void setup(void) {
    ball.x = PLAYER_START_X;
    ball.y = PLAYER_START_Y;
    ball.width = 16;
    ball.height = 16;
    ball.direction = 0;

    load_map();
}

bool check_wall_collision(float x, float y, float width, float height) {
    int left_tile = floor(x / TILE_SIZE);
    int right_tile = floor((x + width) / TILE_SIZE);
    int top_tile = floor(y / TILE_SIZE);
    int bottom_tile = floor((y + height) / TILE_SIZE);

    for (int i = left_tile; i <= right_tile; ++i) {
        for (int j = top_tile; j <= bottom_tile; ++j) {
            if (game_map[i][j] == 1) {
                return true;
            }
        }
    }
    return false;
}

void update(int *input_update) {
    int x = 0;
    int y = 0;
    int flag = 0;
    //contains how many ticks are left before the next second
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    //caps frame rate
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    float tempX{ball.x};
    float tempY{ball.y};
    int tempDirect = 0;

    while (*input_update != '0') {
        switch (*input_update) {
            case 'a':
                ball.y -= BALL_SPEED * cos(ball.radDirection);
                ball.x += BALL_SPEED * sin(ball.radDirection);
                break;
            case 'w':
                ball.y -= BALL_SPEED * cos(ball.radDirection + ((90 * 3.1415926535) / 180));
                ball.x += BALL_SPEED * sin(ball.radDirection + ((90 * 3.1415926535) / 180));                
                break;
            case 'd':
                ball.y -= BALL_SPEED * cos(ball.radDirection + ((180 * 3.1415926535) / 180));
                ball.x += BALL_SPEED * sin(ball.radDirection + ((180 * 3.1415926535) / 180));                
                break;
            case 's':
                ball.y -= BALL_SPEED * cos(ball.radDirection - ((90 * 3.1415926535) / 180));
                ball.x += BALL_SPEED * sin(ball.radDirection - ((90 * 3.1415926535) / 180));
                break;
            case 'r':
                ball.direction += TURN_AMOUNT;
                tempDirect = static_cast<int>(ball.direction) % 360;
                if (tempDirect < 0) {
                    tempDirect += 360;
                }
                ball.direction = static_cast<float>(tempDirect);
                ball.radDirection = (ball.direction / 180) * 3.1415926535;
                break;
            case 'l':
                ball.direction -= TURN_AMOUNT;
                tempDirect = static_cast<int>(ball.direction) % 360;
                if (tempDirect < 0) {
                    tempDirect += 360;
                }
                ball.direction = static_cast<float>(tempDirect);
                ball.radDirection = (ball.direction / 180) * 3.1415926535;
                break;
        }
        
        if (check_wall_collision(ball.x, tempY, ball.width, ball.height)) {
            ball.x = tempX;
        } else {
            tempX = ball.x;
        }

        if (check_wall_collision(tempX, ball.y, ball.width, ball.height)) {
            ball.y = tempY;
        } else {
            tempY = ball.y;
        }

        input_update++;
    }

    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0;

    last_frame_time = SDL_GetTicks();
}

void render(void) {
    int x = 0;
    int y = 0;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int j = 0; j < MAP_HEIGHT; j++) {
            if (game_map[i][j] == 1) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect tile_rect = { i * TILE_SIZE, j * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderFillRect(renderer, &tile_rect);
            }
        }
    }

    SDL_Rect ball_rect = {
        (int)ball.x,
        (int)ball.y,
        (int)ball.width,
        (int)ball.height
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &ball_rect);

    float midX = ball.x + ball.width / 2;
    float midY = ball.y + ball.height / 2;

    float rayX = midX;
    float rayY = midY;
    float rayStep = 1.0;
    bool collide = false;
    float step = 1;
    float pos = 0;
    int ray_dist = 255;

    for (float g = -1; g < 1; g+= 0.005) {
        rayX = midX;
        rayY = midY;
        rayStep = 1;
        collide = false;
        ray_dist = 255;

        while (rayX >= 0 && rayX < WINDOW_WIDTH && rayY >= 0 && rayY < WINDOW_HEIGHT) {
            rayX += rayStep * cos(ball.radDirection + g);
            rayY += rayStep * sin(ball.radDirection + g);
            collide = check_wall_collision(rayX, rayY, 1.0, 1.0);
            if (collide) {
                pos = (step / 402);
                pos = 800 * pos;
                SDL_SetRenderDrawColor(renderer, ray_dist, ray_dist, ray_dist, ray_dist);
                SDL_RenderDrawLine(renderer, pos, 100, pos, 10);
                printf("ray_dist: %d\n", ray_dist);
                break;
            }
            if (ray_dist > 0) {
                ray_dist--;
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, midX, midY, rayX, rayY);
    
        step += 1;
    }

    SDL_RenderPresent(renderer);
}

void destroy_window(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* args[]) {
    game_is_running = initialize_window();
    int * pInput_update {input_update};

    setup();
    TTF_Init();

    getcwd(cwd, sizeof(cwd));
    pPath = strcat(cwd, "/Roboto-Black.ttf");

    while (game_is_running) {
        pInput_update = process_input(input_update);
        update(pInput_update);
        render();
    }
    	
    TTF_Quit();
    destroy_window();

    return 0;
}
