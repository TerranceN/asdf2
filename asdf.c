#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include <stdio.h>

bool isWhitespace(char c) {
  return c == ' ' || c == '\0';
}

int main() {
  initscr();
  noecho();
  keypad(stdscr, TRUE);

  const int BUFFER_SIZE = 150;

  char* buffer = malloc(BUFFER_SIZE*sizeof(char));
  buffer[0] = '\0';
  int size = 0;
  int cursor = 0;

  while(1) {
    int c = getch();
    switch(c) {
      case KEY_BACKSPACE:
      case KEY_DC:
      case 127:
        if (size > 0) {
          memmove(buffer+cursor-1, buffer+cursor, (size-cursor+1)*sizeof(char));
          size--;
          if (cursor > 0) {
            cursor--;
          }
        }
        break;
      case KEY_LEFT:
        if (cursor > 0) {
          cursor--;
        }
        break;
      case KEY_RIGHT:
        if (cursor < size) {
          cursor++;
        }
        break;
      case 562:
        cursor++;
        while (cursor < size && !(!isWhitespace(buffer[cursor]) && isWhitespace(buffer[cursor+1]))) {
          cursor++;
        }
        cursor++;
        if (cursor > size) {
          cursor = size;
        }
        break;
      case 547:
        cursor -= 2;
        while (cursor > 0 && !(isWhitespace(buffer[cursor]) && !isWhitespace(buffer[cursor+1]))) {
          cursor--;
        }
        if (cursor > 0) {
          cursor++;
        }
        if (cursor < 0) {
          cursor = 0;
        }
        break;
      default:
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ') {
          if (size < BUFFER_SIZE-1) {
            memmove(buffer+cursor+1, buffer+cursor, (size-cursor+1)*sizeof(char));
            buffer[cursor] = c;
            size++;
            cursor++;
          }
        }
        //sprintf(buffer, "Num: %d", c);
        break;
    }

    clear();

    mvprintw(0, 0, buffer);
    move(0, cursor);
  }

  free(buffer);

  endwin();

  return 0;
}
