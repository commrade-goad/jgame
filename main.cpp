#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <random>
#include <curses.h>
#include <chrono>

const size_t BOARD_X = 40;
const size_t BOARD_Y = 10;
const char *BORDER_CHAR = "#";

enum ProgMode {
    Game,
    Menu
};

enum PlayerMode {
    Jump,
    Walk
};

struct Player {
    size_t x;
    size_t y;
    PlayerMode mode;
};

struct Coordinate {
    size_t x;
    size_t y;
};

float gen_random_float(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distr(min, max);
    return distr(gen);
}

int gen_random_int(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distr(min, max);
    return distr(gen);
}

void init_curses_sc() {
    initscr();
    raw();
    curs_set(0);
    nodelay(stdscr, true);
}

void close_curses_screen(void) {
    endwin();
}

size_t game(ProgMode mode) {
    size_t score = 0;
    init_curses_sc();
    int ch;

    Player player = {3, BOARD_Y - 2, Walk};
    std::vector<Coordinate> enemies;

    auto jump_start = std::chrono::high_resolution_clock::now();
    auto jump_cooldown_start = std::chrono::high_resolution_clock::now();
    auto enemy_speed_s = std::chrono::high_resolution_clock::now();
    auto enemy_spawn_s = std::chrono::high_resolution_clock::now();

    timeout(0);
    while (mode == Game) {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto jump_duration = std::chrono::duration_cast<std::chrono::duration<double>>(current_time - jump_start);
        auto jump_cooldown = std::chrono::duration_cast<std::chrono::duration<double>>(current_time - jump_cooldown_start);
        auto enemy_speed = std::chrono::duration_cast<std::chrono::duration<double>>(current_time - enemy_speed_s);
        auto enemy_spawn_duration = std::chrono::duration_cast<std::chrono::duration<double>>(current_time - enemy_spawn_s);

        if (player.mode == Jump && jump_duration.count() >= 0.5) {
            player.y += 1;
            player.mode = Walk;
        }

        if (enemy_speed.count() >= 0.1) {
            for (auto &enemy : enemies) {
                if (enemy.x > 0) {
                    enemy.x -= 1;
                } else {
                    enemies.erase(enemies.begin());
                    ++score;
                }
            }
            enemy_speed_s = current_time;
        }

        if (enemy_spawn_duration.count() >= gen_random_float(0.8, 2)) {
            Coordinate newEnemy = {BOARD_X - 2 + gen_random_int(0, 3), BOARD_Y - 2};
            enemies.push_back(newEnemy);
            enemy_spawn_s = current_time;
        }

        for (size_t y = 0; y < BOARD_Y; y++) {
            for (size_t x = 0; x < BOARD_X; x++) {
                if (y == 0 || y == BOARD_Y - 1 || x == 0 || x == BOARD_X - 1)
                    mvprintw(y, x, "%s\n", BORDER_CHAR);
                else if (x == player.x && y == player.y)
                    mvprintw(y, x, "%s\n", "*");
                else {
                    for (const auto &enemy : enemies) {
                        if (x == enemy.x && y == enemy.y) {
                            mvprintw(y, x, "%s\n", "W");
                            break;
                        }
                        if (player.x == enemy.x && player.y == enemy.y) {
                            mode = Menu;
                            break;
                        }
                    }
                }
            }
        }
        mvprintw(0, 3, " SCORE : %lu ", score);
        refresh();
        ch = getch();

        if (ch == 'q') {
            break;
        } else if (ch == 'e' && player.mode != Jump && jump_cooldown.count() >= 0.15) {
            player.y -= 1;
            player.mode = Jump;
            jump_start = current_time;
            jump_cooldown_start = current_time;
        }
    }

    close_curses_screen();
    return score;
}

void menu(ProgMode mode) {
    size_t high_score;
    init_curses_sc();
    int ch;
    while (mode == Menu) {
        mvprintw(BOARD_Y/2-3, (BOARD_X-strlen("HIGH SCORE : "))/2, "HIGH SCORE : %lu", high_score);
        mvprintw(BOARD_Y/2-2, (BOARD_X-strlen("THE GAME OF ALL TIME"))/2, "THE GAME OF ALL TIME");
        mvprintw(BOARD_Y/2, (BOARD_X-strlen("(q) quit"))/2, "(q) quit");
        mvprintw(BOARD_Y/2-1, (BOARD_X-strlen("(p) play"))/2, "(p) play");
        refresh();
        ch = getch();

        if (ch == 'q') {
            close_curses_screen();
            exit(0);
        } else if (ch == 'p') {
            mode = Game;
            size_t final_score = game(mode);
            close_curses_screen();
            if (final_score > high_score) {
                high_score = final_score;
            }
        }
    }
}

int main (void) {
    ProgMode mode = Menu;
    bool running = true;
    while (running) {
        menu(mode);
    }
    return 0;
}

