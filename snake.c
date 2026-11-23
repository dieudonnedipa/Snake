/*
 * snake.c - Console Snake for Windows (C + conio.h)
 * Compile: gcc snake.c -o snake.exe
 * Run:     .\snake.exe
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define WIDTH        40
#define HEIGHT       20
#define MAX_LEN      ((WIDTH - 2) * (HEIGHT - 2))
#define START_DELAY  150
#define MIN_DELAY    60
#define SCORE_FILE   "highscore.txt"

typedef struct { int x, y; } Point;
typedef enum { UP, DOWN, LEFT, RIGHT } Direction;
enum { DEAD, ALIVE, WON, QUIT };

typedef struct {
    Point body[MAX_LEN];
    int length;
    Direction dir;
    Direction moved_dir;
} Snake;

static Snake snake;
static Point food;
static int score = 0;
static int high_score = 0;
static int delay_ms = START_DELAY;

static const int DX[] = { 0, 0, -1, 1 };
static const int DY[] = { -1, 1, 0, 0 };


static void set_cursor_visible(int visible) {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 1;
    info.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

static void move_cursor(int x, int y) {
    COORD pos;
    pos.X = (SHORT)x;
    pos.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

static void clear_input(void) {
    while (_kbhit()) _getch();
}


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
        food.x = 1 + rand() % (WIDTH - 2);
        food.y = 1 + rand() % (HEIGHT - 2);
    } while (on_snake(food.x, food.y));
}

static void init_game(void) {
    snake.length = 3;
    snake.dir = RIGHT;
    snake.moved_dir = RIGHT;
    for (int i = 0; i < snake.length; i++) {
        snake.body[i].x = WIDTH / 2 - i;
        snake.body[i].y = HEIGHT / 2;
    }
    score = 0;
    delay_ms = START_DELAY;
    place_food();
}


static void draw(void) {
    char grid[HEIGHT][WIDTH];
    char out[HEIGHT * (WIDTH + 1) + 1];
    int pos = 0;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int border = (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1);
            grid[y][x] = border ? '#' : ' ';
        }
    }
    grid[food.y][food.x] = '*';
    for (int i = snake.length - 1; i > 0; i--) {
        grid[snake.body[i].y][snake.body[i].x] = 'o';
    }
    grid[snake.body[0].y][snake.body[0].x] = 'O';

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) out[pos++] = grid[y][x];
        out[pos++] = '\n';
    }
    out[pos] = '\0';

    move_cursor(0, 0);
    fputs(out, stdout);
    printf("Score: %d   High score: %d   Length: %d   \n",
           score, high_score, snake.length);
    printf("Move: arrows or WASD   Pause: P   Quit: Q   ");
    fflush(stdout);
}


static int is_opposite(Direction a, Direction b) {
    return (a ^ 1) == b;
}

static void turn(Direction wanted) {
    if (!is_opposite(wanted, snake.moved_dir)) snake.dir = wanted;
}

static void pause_game(void) {
    move_cursor(0, HEIGHT + 2);
    printf("PAUSED - press any key to continue");
    fflush(stdout);
    _getch();
    clear_input();
    move_cursor(0, HEIGHT + 2);
    printf("                                  ");
}

static int handle_input(void) {
    while (_kbhit()) {
        int ch = _getch();
        if (ch == 0 || ch == 224) {
            switch (_getch()) {
                case 72: turn(UP);    break;
                case 80: turn(DOWN);  break;
                case 75: turn(LEFT);  break;
                case 77: turn(RIGHT); break;
            }
        } else {
            switch (ch) {
                case 'w': case 'W': turn(UP);    break;
                case 's': case 'S': turn(DOWN);  break;
                case 'a': case 'A': turn(LEFT);  break;
                case 'd': case 'D': turn(RIGHT); break;
                case 'p': case 'P': pause_game(); break;
                case 'q': case 'Q': case 27: return 1;
            }
        }
    }
    return 0;
}


static int update(void) {
    Point head = snake.body[0];
    head.x += DX[snake.dir];
    head.y += DY[snake.dir];
    snake.moved_dir = snake.dir;

    if (head.x <= 0 || head.x >= WIDTH - 1 ||
        head.y <= 0 || head.y >= HEIGHT - 1) return DEAD;

    int eating = (head.x == food.x && head.y == food.y);

    int check = eating ? snake.length : snake.length - 1;
    for (int i = 0; i < check; i++) {
        if (snake.body[i].x == head.x && snake.body[i].y == head.y) return DEAD;
    }

    if (eating) snake.length++;
    for (int i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }
    snake.body[0] = head;

    if (eating) {
        score += 10;
        delay_ms = START_DELAY - (score / 10) * 4;
        if (delay_ms < MIN_DELAY) delay_ms = MIN_DELAY;
        if (snake.length == MAX_LEN) return WON;
        place_food();
    }
    return ALIVE;
}

static int play(void) {
    int result = ALIVE;
    system("cls");
    while (result == ALIVE) {
        draw();
        Sleep(delay_ms);
        if (handle_input()) return QUIT;
        result = update();
    }
    return result;
}


static void show_menu(void) {
    system("cls");
    printf("\n\n");
    printf("      ==============================\n");
    printf("                S N A K E\n");
    printf("      ==============================\n\n");
    printf("      Eat the * to grow. Avoid the walls\n");
    printf("      and your own tail.\n\n");
    printf("      Controls: arrows or WASD\n");
    printf("      Pause: P     Quit: Q\n\n");
    printf("      High score: %d\n\n", high_score);
    printf("      Press any key to start...");
    fflush(stdout);
    clear_input();
    _getch();
}

static int game_over_screen(int result) {
    update_high_score();
    move_cursor(0, HEIGHT + 2);
    if (result == WON) printf("YOU WIN! The board is full!          \n");
    else               printf("GAME OVER! Final score: %d           \n", score);
    printf("Press R to play again or Q to quit.   ");
    fflush(stdout);
    clear_input();
    for (;;) {
        int ch = _getch();
        if (ch == 'r' || ch == 'R') return 1;
        if (ch == 'q' || ch == 'Q' || ch == 27) return 0;
    }
}

int main(void) {
    srand((unsigned)time(NULL));
    set_cursor_visible(0);
    high_score = load_high_score();
    show_menu();

    int again = 1;
    while (again) {
        init_game();
        int result = play();
        if (result == QUIT) {
            update_high_score();
            break;
        }
        again = game_over_screen(result);
    }

    system("cls");
    set_cursor_visible(1);
    printf("Thanks for playing! Final high score: %d\n", high_score);
    return 0;
}