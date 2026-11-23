/*
 * snake_gfx.c - Graphical Snake with raylib (C)
 * Compile (MSYS2 UCRT64):
 *   gcc -Wall -Wextra snake_gfx.c -o snake_gfx.exe -lraylib -lopengl32 -lgdi32 -lwinmm
 */
#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define COLS         30
#define ROWS         20
#define CELL         24
#define HUD_HEIGHT   48
#define SCREEN_W     (COLS * CELL)
#define SCREEN_H     (ROWS * CELL + HUD_HEIGHT)
#define MAX_LEN      (COLS * ROWS)
#define QUEUE_SIZE   2
#define START_STEP   0.14f
#define MIN_STEP     0.06f
#define SCORE_FILE   "highscore.txt"

typedef struct { int x, y; } Cell;
typedef enum { UP, DOWN, LEFT, RIGHT } Direction;
typedef enum { STATE_MENU, STATE_PLAYING, STATE_PAUSED,
               STATE_GAME_OVER, STATE_WON } GameState;
enum { STEP_ALIVE, STEP_DEAD, STEP_WON };

typedef struct {
    Cell body[MAX_LEN];
    int length;
    Direction dir;
    Direction queue[QUEUE_SIZE];
    int queued;
} Snake;

static Snake snake;
static Cell food;
static int score = 0;
static int high_score = 0;
static float step_time = START_STEP;
static float timer = 0.0f;

static const int DX[] = { 0, 0, -1, 1 };
static const int DY[] = { -1, 1, 0, 0 };

static const Color COL_GRASS_A = { 110, 155,  60, 255 };
static const Color COL_GRASS_B = { 102, 147,  52, 255 };
static const Color COL_HUD     = {  40,  52,  44, 255 };
static const Color COL_HEAD    = {  22, 120,  60, 255 };
static const Color COL_TAIL    = { 120, 210, 120, 255 };
static const Color COL_FOOD    = { 230,  57,  70, 255 };


static int load_high_score(void) {
    int value = 0;
    FILE *f = fopen(SCORE_FILE, "r");
    if (f != NULL) {
        if (fscanf(f, "%d", &value) != 1) value = 0;
        fclose(f);
    }
    return value;
}

static void save_high_score(int value) {
    FILE *f = fopen(SCORE_FILE, "w");
    if (f != NULL) {
        fprintf(f, "%d\n", value);
        fclose(f);
    }
}

static void update_high_score(void) {
    if (score > high_score) {
        high_score = score;
        save_high_score(high_score);
    }
}


static int on_snake(int x, int y) {
    for (int i = 0; i < snake.length; i++) {
        if (snake.body[i].x == x && snake.body[i].y == y) return 1;
    }
    return 0;
}

static void place_food(void) {
    do {
        food.x = GetRandomValue(0, COLS - 1);
        food.y = GetRandomValue(0, ROWS - 1);
    } while (on_snake(food.x, food.y));
}

static void init_game(void) {
    snake.length = 3;
    snake.dir = RIGHT;
    snake.queued = 0;
    for (int i = 0; i < snake.length; i++) {
        snake.body[i].x = COLS / 2 - i;
        snake.body[i].y = ROWS / 2;
    }
    score = 0;
    step_time = START_STEP;
    timer = 0.0f;
    place_food();
}


static int is_opposite(Direction a, Direction b) {
    return (a ^ 1) == b;
}

static void queue_turn(Direction wanted) {
    Direction last = (snake.queued > 0) ? snake.queue[snake.queued - 1]
                                        : snake.dir;
    if (wanted == last || is_opposite(wanted, last)) return;
    if (snake.queued < QUEUE_SIZE) snake.queue[snake.queued++] = wanted;
}

static void read_input(void) {
    if (IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W)) queue_turn(UP);
    if (IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S)) queue_turn(DOWN);
    if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) queue_turn(LEFT);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) queue_turn(RIGHT);
}


static int step_snake(void) {
    if (snake.queued > 0) {
        snake.dir = snake.queue[0];
        snake.queue[0] = snake.queue[1];
        snake.queued--;
    }

    Cell head = snake.body[0];
    head.x += DX[snake.dir];
    head.y += DY[snake.dir];

    if (head.x < 0 || head.x >= COLS || head.y < 0 || head.y >= ROWS)
        return STEP_DEAD;

    int eating = (head.x == food.x && head.y == food.y);

    int check = eating ? snake.length : snake.length - 1;
    for (int i = 0; i < check; i++) {
        if (snake.body[i].x == head.x && snake.body[i].y == head.y)
            return STEP_DEAD;
    }

    if (eating) snake.length++;
    for (int i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }
    snake.body[0] = head;

    if (eating) {
        score += 10;
        step_time = START_STEP - (score / 10) * 0.003f;
        if (step_time < MIN_STEP) step_time = MIN_STEP;
        if (snake.length == MAX_LEN) return STEP_WON;
        place_food();
    }
    return STEP_ALIVE;
}

static GameState update_playing(void) {
    read_input();
    if (IsKeyPressed(KEY_P)) return STATE_PAUSED;

    timer += GetFrameTime();
    if (timer >= step_time) {
        timer -= step_time;
        int result = step_snake();
        if (result == STEP_DEAD) { update_high_score(); return STATE_GAME_OVER; }
        if (result == STEP_WON)  { update_high_score(); return STATE_WON; }
    }
    return STATE_PLAYING;
}


static Color lerp_color(Color a, Color b, float t) {
    Color c;
    c.r = (unsigned char)(a.r + (b.r - a.r) * t);
    c.g = (unsigned char)(a.g + (b.g - a.g) * t);
    c.b = (unsigned char)(a.b + (b.b - a.b) * t);
    c.a = 255;
    return c;
}

static void draw_board(void) {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            Color c = ((x + y) % 2 == 0) ? COL_GRASS_A : COL_GRASS_B;
            DrawRectangle(x * CELL, HUD_HEIGHT + y * CELL, CELL, CELL, c);
        }
    }
}

static void draw_food(void) {
    float pulse = sinf((float)GetTime() * 6.0f) * 1.5f;
    float cx = food.x * CELL + CELL / 2.0f;
    float cy = HUD_HEIGHT + food.y * CELL + CELL / 2.0f;
    DrawCircleV((Vector2){ cx, cy }, CELL / 2.0f - 3.0f + pulse, COL_FOOD);
    DrawCircleV((Vector2){ cx - 3, cy - 3 }, 2.5f, Fade(WHITE, 0.6f));
}

static void draw_snake(void) {
    for (int i = snake.length - 1; i >= 0; i--) {
        float t = (snake.length > 1) ? (float)i / (snake.length - 1) : 0.0f;
        Color c = lerp_color(COL_HEAD, COL_TAIL, t);
        Rectangle r = { snake.body[i].x * CELL + 1.0f,
                        HUD_HEIGHT + snake.body[i].y * CELL + 1.0f,
                        CELL - 2.0f, CELL - 2.0f };
        DrawRectangleRounded(r, 0.4f, 6, c);
    }

    /* eyes: one step forward, one step to each side of the head */
    float cx = snake.body[0].x * CELL + CELL / 2.0f;
    float cy = HUD_HEIGHT + snake.body[0].y * CELL + CELL / 2.0f;
    float fx = (float)DX[snake.dir], fy = (float)DY[snake.dir];
    float px = -fy, py = fx;                       /* perpendicular vector */
    for (int side = -1; side <= 1; side += 2) {
        Vector2 eye = { cx + fx * 4 + px * 5 * side, cy + fy * 4 + py * 5 * side };
        DrawCircleV(eye, 3.5f, WHITE);
        DrawCircleV((Vector2){ eye.x + fx, eye.y + fy }, 1.7f, BLACK);
    }
}

static void draw_centered(const char *text, int y, int size, Color c) {
    DrawText(text, (SCREEN_W - MeasureText(text, size)) / 2, y, size, c);
}

static void draw_hud(void) {
    DrawRectangle(0, 0, SCREEN_W, HUD_HEIGHT, COL_HUD);
    DrawText(TextFormat("Score: %d", score), 16, 12, 24, WHITE);
    const char *best = TextFormat("Best: %d", high_score);
    DrawText(best, SCREEN_W - MeasureText(best, 24) - 16, 12, 24, GOLD);
}

static void draw_overlay(const char *title, const char *hint, Color c) {
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.6f));
    draw_centered(title, SCREEN_H / 2 - 60, 48, c);
    draw_centered(hint, SCREEN_H / 2 + 10, 22, RAYWHITE);
}

static void draw_game(GameState state) {
    draw_board();
    draw_food();
    draw_snake();
    draw_hud();

    switch (state) {
        case STATE_MENU:
            draw_overlay("SNAKE", "Press ENTER to play  -  Arrows/WASD move, P pause", GREEN);
            break;
        case STATE_PAUSED:
            draw_overlay("PAUSED", "Press P to continue", YELLOW);
            break;
        case STATE_GAME_OVER:
            draw_overlay("GAME OVER", TextFormat("Score %d  -  Press ENTER to retry", score), RED);
            break;
        case STATE_WON:
            draw_overlay("YOU WIN!", "The board is full! Press ENTER", GOLD);
            break;
        default:
            break;
    }
}

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "Snake - raylib");
    SetTargetFPS(60);
    high_score = load_high_score();
    init_game();

    GameState state = STATE_MENU;
    while (!WindowShouldClose()) {
        switch (state) {
            case STATE_MENU:
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    init_game();
                    state = STATE_PLAYING;
                }
                break;
            case STATE_PLAYING:
                state = update_playing();
                break;
            case STATE_PAUSED:
                if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_SPACE)) state = STATE_PLAYING;
                break;
            case STATE_GAME_OVER:
            case STATE_WON:
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_R)) {
                    init_game();
                    state = STATE_PLAYING;
                }
                break;
        }

        BeginDrawing();
        draw_game(state);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}