#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include <stdio.h>

const int MAX_HISTORY_LINE_SIZE=300;

bool isWhitespace(char c) {
  return c == ' ' || c == '\0';
}

char* loadHistoryData(int* numLines) {
  char* history_data;

  FILE* history_file = fopen(getenv("HISTFILE"), "r");
  if (history_file != NULL) {
    int nLines = 0;
    {
      char line[MAX_HISTORY_LINE_SIZE];
      while(fgets((char*)(&line), MAX_HISTORY_LINE_SIZE*sizeof(char), history_file) != NULL) {
        nLines += 1;
      }
    }

    rewind(history_file);

    {
      history_data = malloc(nLines*MAX_HISTORY_LINE_SIZE*sizeof(char));
      for (int i = 0; i < nLines; i++) {
        fgets(history_data+i*MAX_HISTORY_LINE_SIZE, MAX_HISTORY_LINE_SIZE*sizeof(char), history_file);
      }
    }

    *numLines = nLines;
  }
  fclose(history_file);

  return history_data;
}

int main() {
  initscr();
  noecho();
  keypad(stdscr, TRUE);

  const int BUFFER_SIZE = 150;

  char* input = malloc(BUFFER_SIZE*sizeof(char));
  input[0] = '\0';
  int size = 0;
  int cursor = 0;

  int numLines;
  char* historyData = loadHistoryData(&numLines);

  int selected = 0;

  while(1) {
    int c = getch();
    switch(c) {
      case KEY_BACKSPACE:
      case KEY_DC:
      case 127:
        if (size > 0) {
          memmove(input+cursor-1, input+cursor, (size-cursor+1)*sizeof(char));
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
      case KEY_UP:
        if (selected > 0) {
          selected--;
        }
        break;
      case KEY_DOWN:
        selected++;
        break;
      case 562:
        cursor++;
        while (cursor < size && !(!isWhitespace(input[cursor]) && isWhitespace(input[cursor+1]))) {
          cursor++;
        }
        cursor++;
        if (cursor > size) {
          cursor = size;
        }
        break;
      case 547:
        cursor -= 2;
        while (cursor > 0 && !(isWhitespace(input[cursor]) && !isWhitespace(input[cursor+1]))) {
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
        if ((c >= 'a' && c <= 'z')
         || (c >= 'A' && c <= 'Z')
         || c == ' '
         || c == '-'
         || c == '.'
         || c == '*'
         || c == '\\') {
          if (size < BUFFER_SIZE-1) {
            memmove(input+cursor+1, input+cursor, (size-cursor+1)*sizeof(char));
            input[cursor] = c;
            size++;
            cursor++;
          }
        }
        //sprintf(input, "Num: %d", c);
        break;
    }

    clear();

    int max_y;
    int max_x;
    getmaxyx(stdscr, max_y, max_x);

    mvprintw(0, 0, input);

    for (int i = 0; i < max_x; i++) {
      mvprintw(1, i, "-");
    }

    int line = 2;
    for (int i = numLines; i >= 0 && line < 50; i--) {
      mvprintw(line, 0, " ");
      if (strstr(historyData+i*MAX_HISTORY_LINE_SIZE, input) != NULL) {
        mvprintw(line, 1, historyData+i*MAX_HISTORY_LINE_SIZE);
        line++;
      }
    }
    mvprintw(selected+2, 0, ">");

    move(0, cursor);
  }

  free(historyData);
  free(input);

  endwin();

  return 0;
}
