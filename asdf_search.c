#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include <stdio.h>

const int MAX_HISTORY_LINE_SIZE=300;

void outputCommand(char* fileName, char* command) {
  FILE* outputFile = fopen(fileName, "w");
  if (outputFile == NULL) {
    printf("Can't open output file.\n" );
    exit(1);
  }
  fputs(command, outputFile);
  fclose(outputFile);
}

void streamCommand(char* command) {
  FILE *fp;
  char path[1035];

  /* Open the command for reading. */
  fp = popen(command, "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    printf("%s", path);
  }

  /* close */
  pclose(fp);
}

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

void searchHistory(char* input, char* lines, bool* renderLine, int numLines) {
  for (int i = numLines; i >= 0; i--) {
    if (*input == '\0' || strstr(lines+i*MAX_HISTORY_LINE_SIZE, input) != NULL) {
      renderLine[i] = true;
    } else {
      renderLine[i] = false;
    }
  }
}

void renderScreen(char* input, int cursor, int selected, char* lines, bool* renderLine, int numLines) {
  clear();

  int max_y;
  int max_x;
  getmaxyx(stdscr, max_y, max_x);

  mvprintw(0, 0, input);

  for (int i = 0; i < max_x; i++) {
    mvprintw(1, i, "-");
  }

  int line = 2;
  for (int i = numLines-1; i >= 0 && line < 50; i--) {
    mvprintw(line, 0, " ");
    if (renderLine[i]) {
      mvprintw(line, 1, lines+i*MAX_HISTORY_LINE_SIZE);
      line++;
    }
  }
  mvprintw(selected+2, 0, ">");

  move(0, cursor);
}

int main(int argc, char** argv) {
  int result = 0;
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

  bool* renderLine = malloc(numLines*sizeof(bool));
  memset(renderLine, 0, numLines*sizeof(bool));

  int selected = 0;

  bool runCommand = false;

  searchHistory(input, historyData, renderLine, numLines);
  renderScreen(input, cursor, selected, historyData, renderLine, numLines);

  while(!runCommand) {
    bool inputChanged = false;
    int c = getch();
    switch(c) {
      case KEY_BACKSPACE:
      case KEY_DC:
      case 127:
        if (size > 0) {
          inputChanged = true;
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
      case 10:
        runCommand = true;
        result = 0;
        break;
      case 0:
        runCommand = true;
        result = 1;
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
            inputChanged = true;
            memmove(input+cursor+1, input+cursor, (size-cursor+1)*sizeof(char));
            input[cursor] = c;
            size++;
            cursor++;
          }
        } else {
          //sprintf(input, "Num: %d", c);
        }
        break;
    }

    if (inputChanged) {
      searchHistory(input, historyData, renderLine, numLines);
    }

    renderScreen(input, cursor, selected, historyData, renderLine, numLines);
  }

  endwin();

  {
    int found = 0;
    for (int i = numLines-1; i >= 0; i--) {
      if (strstr(historyData+i*MAX_HISTORY_LINE_SIZE, input) != NULL) {
        found++;
        if (found > selected) {
          char* tmp = malloc(MAX_HISTORY_LINE_SIZE*sizeof(char));
          strcpy(tmp, historyData+i*MAX_HISTORY_LINE_SIZE);
          strtok(tmp, "\n");
          //printf("%s", tmp);
          outputCommand(argv[1], tmp);
          free(tmp);
          break;
        }
      }
    }
  }

  free(historyData);
  free(input);

  return result;
}
