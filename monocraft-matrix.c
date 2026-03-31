#define _XOPEN_SOURCE_EXTENDED
#include <locale.h>
#include <ncursesw/curses.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define DELAY 50000
#define GALACTIC_START 0xEB40
#define GALACTIC_RANGE 26
#define SPAWN_CHANCE 3
#define TAIL_LENGTH 15

volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) { keep_running = 0; }

int main() {
  setlocale(LC_ALL, "");
  initscr();
  noecho();
  curs_set(FALSE);
  start_color();

  init_pair(1, COLOR_BLUE, COLOR_BLACK);
  init_pair(2, COLOR_CYAN, COLOR_BLACK);

  srand(time(NULL));

  // Catch Ctrl+C
  signal(SIGINT, handle_sigint);

  // Initialize dimensions and dynamically allocate memory
  int width = 0, height = 0;
  getmaxyx(stdscr, height, width);

  int current_capacity = width;
  int *drops = malloc(sizeof(int) * current_capacity);
  for (int ii = 0; ii < current_capacity; ii++) {
    drops[ii] = -1;
  }

  // Loop runs until Ctrl+C sets keep_running to 0
  while (keep_running) {
    int new_height, new_width;
    getmaxyx(stdscr, new_height, new_width);

    if (new_width > current_capacity) {
      int *new_drops = realloc(drops, sizeof(int) * new_width);
      if (!new_drops) {
        break;
      }
      drops = new_drops;
      for (int ii = current_capacity; ii < new_width; ii++) {
        drops[ii] = -1;
      }
      current_capacity = new_width;
    }
    height = new_height;
    width = new_width;

    for (int ii = 0; ii < width; ii++) {
      if (drops[ii] == -1) {
        if (rand() % 100 < SPAWN_CHANCE) {
          drops[ii] = 0;
        }
        continue;
      }

      int tail_end = drops[ii] - TAIL_LENGTH;
      if (tail_end >= 0 && tail_end < height) {
        mvaddch(tail_end, ii, ' ');
      }

      if (drops[ii] > 0 && drops[ii] - 1 < height) {
        wchar_t trail_char = GALACTIC_START + (rand() % GALACTIC_RANGE);
        attron(COLOR_PAIR(1));
        mvaddnwstr(drops[ii] - 1, ii, &trail_char, 1);
        attroff(COLOR_PAIR(1));
      }

      if (drops[ii] < height) {
        wchar_t head_char = GALACTIC_START + (rand() % GALACTIC_RANGE);
        attron(COLOR_PAIR(2) | A_BOLD);
        mvaddnwstr(drops[ii], ii, &head_char, 1);
        attroff(COLOR_PAIR(2) | A_BOLD);
      }

      drops[ii]++;
      if (drops[ii] >= height + TAIL_LENGTH) {
        drops[ii] = -1;
      }
    }

    refresh();
    usleep(DELAY);
  }

  // Graceful exit triggered by Ctrl+C
  free(drops);
  endwin();
  return 0;
}
