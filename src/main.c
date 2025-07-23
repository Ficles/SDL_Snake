#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

#define noop

// Define Colour constants
const SDL_Color red = {255, 0, 0, 255};
const SDL_Color d_red = {127, 00, 0, 255};
const SDL_Color blue = {0, 0, 255, 255};
const SDL_Color white = {255, 255, 255, 255};
const SDL_Color black = {0, 0, 0, 255};

int draw_filled_rect(SDL_Renderer* renderer, SDL_Rect* rect, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, rect);
    SDL_SetRenderDrawColor(renderer, black.r, black.g, black.b, black.a); // Set draw colour back to background colour

    return 0;
}

int draw_rect(SDL_Renderer* renderer, SDL_Rect* rect, SDL_Color color, int border_width)
{
    SDL_Rect border_rect = *rect;
    int i = 0;

    // Draw squares of decreasing size inside eachother to replicate a thicker border
    while (i < border_width) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawRect(renderer, &border_rect);
        SDL_SetRenderDrawColor(renderer, black.r, black.g, black.b, black.a);

        border_rect.x++;
        border_rect.y++;
        border_rect.w -= 2;
        border_rect.h -= 2;
        i++;
    }
    
    return 0;
}

int draw_grid(SDL_Renderer* renderer, int square_size)
{
    // Draw vertical lines spaced "square_size" apart until the end of the screen
    int i = square_size; 
    while (i < 1024) {
        SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
        SDL_RenderDrawLine(renderer, i, 0, i, 768);
        i += square_size;
    }

    // Draw horizontal lines spaced "square_size" apart until the end of the screen
    i = square_size;
    while (i < 768) {
        SDL_SetRenderDrawColor(renderer, white.r, white.g, white.b, white.a);
        SDL_RenderDrawLine(renderer, 0, i, 1024, i);
        i += square_size;
    }

    SDL_SetRenderDrawColor(renderer, black.r, black.g, black.b, black.a);
    return 0;
}

// Get current timestamp in millisceonds
long long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}


int main(int argc, char* argv[])
{
    srand(time(NULL)); // Set random number seed

    // Init SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
    }

    // Create SDL window
    SDL_Window* win = SDL_CreateWindow(
        "Snake",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024,
        768,
        0
    );

    Uint32 render_flags = SDL_RENDERER_ACCELERATED; // Enable GPU acceleration;

    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags); // Create renderer object

    // Set renderer draw settings
    SDL_SetRenderDrawColor(rend, black.r, black.g, black.b, black.a);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_ADD);

    // Define variables
    int close = 0;
    SDL_Rect head;
    head.w = 32;
    head.h = 32;
    head.x = 32 * 16;
    head.y = 32 * 12;
    SDL_Rect apple;
    apple.w = 32;
    apple.h = 32;
    apple.x = 32 * 16;
    apple.y = 32 * 12;
    int direction = 2;
    int prev_direction = 2;
    int score = 0;
    int i;
    int touching;
    long long time_of_last_frame = current_timestamp();
    SDL_Rect *tail;
    SDL_Rect *temp_tail;
    // Allocate memory for tail arrays
    tail = calloc(1024*769, sizeof(SDL_Rect));
    temp_tail = calloc(1024*768, sizeof(SDL_Rect));

    // Set apple to random position that is not the same as the head
    while (head.x == apple.x && head.y == apple.y) {
        apple.x = (rand() % 31) * 32;
        apple.y = (rand() % 23) * 32;
    }

    while (!close) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: // Close when window X button pressed
                    close = 1;
                    break;
                case SDL_KEYDOWN:
                    // Set direction with key input
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_W:
                        case SDL_SCANCODE_UP:
                            if (prev_direction != 2) {
                            direction = 0;
                            }
                            break;
                        case SDL_SCANCODE_A:
                        case SDL_SCANCODE_LEFT:
                            if (prev_direction != 3) {
                            direction = 1;
                            }
                            break;
                        case SDL_SCANCODE_S:
                        case SDL_SCANCODE_DOWN:
                            if (prev_direction != 0) {
                            direction = 2;
                            }
                            break;
                        case SDL_SCANCODE_D:
                        case SDL_SCANCODE_RIGHT:
                            if (prev_direction != 1) {
                            direction = 3;
                            }
                            break;
                    }
            }
        }

        // Shift all items in tail array to the right and destroy right-most value
        memcpy(temp_tail, tail, (score+1) * sizeof(SDL_Rect));
        memcpy(&tail[1], temp_tail, (score+1) * sizeof(SDL_Rect));

        // Set first tail value to a copy of the head
        memcpy(tail, &head, sizeof(SDL_Rect));

        // Move head
        switch (direction) {
            case 0:
                head.y -= 32;
                break;
            case 1:
                head.x -= 32;
                break;
            case 2:
                head.y += 32;
                break;
            case 3:
                head.x += 32;
                break;
        }

        // Detect if the head is overlapping the apple
        if (head.x == apple.x && head.y == apple.y) {
            score++;
            touching = 1;

            // Set the apple to a new random position that is not overlapping the snake
            while (touching) {
                apple.x = (rand() % 31) * 32;
                apple.y = (rand() % 23) * 32;

                i = 0;
                touching = 0;
                while (i < score+2) {
                    if (apple.x == tail[i].x && apple.y == tail[i].y) {
                        touching = 1;
                    }
                    i++;
                }
                if (apple.x == head.x && apple.y == head.y) {
                    touching = 1;
                }
            }
        }

        // Detect if the head is overlapping the tail
        i = 0;
        while (i < score+2) {
            if (head.x == tail[i].x && head.y == tail[i].y) {
                close = 1;
            }
            i++;
        }

        // Detect if the head is out of bonds
        if (head.x < 0 || head.y < 0) {
            close = 1;
        } 
        if (head.x > 1024 || head.y > 768) {
            close = 1;
        }

        SDL_RenderClear(rend); // Wipe screen

        draw_filled_rect(rend, &apple, blue); // Draw apple

        // Draw tail
        i = 0;
        while (i < score+2) {
            draw_filled_rect(rend, &tail[i], d_red);
            i++;
        }

        draw_filled_rect(rend, &head, red); // Draw head

        draw_grid(rend, 32); // Draw grid

        SDL_RenderPresent(rend); // Update window

        prev_direction = direction;

        // Ensure FPS is 5
        while (current_timestamp() < (time_of_last_frame + 1000/5)) {}
        time_of_last_frame = current_timestamp();
    }

    printf("Score: %d\n", score); // Announce final score

    // Clear memory
    free(tail);
    free(temp_tail);

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}