#define _GNU_SOURCE
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include <stdio.h>

#include "strmap.h"

/* Return code meanings:
 * 10: run the selected command
 * 11: let the user modify the selected command
 */

typedef struct {
  int start;
  int size;
} Match;

const int MAX_SEARCH_SIZE = 150;
const int MAX_HISTORY_LINE_SIZE=300;

void outputCommand(char* fileName, char* command) {
  FILE* outputFile = fopen(fileName, "w");
  if (outputFile == NULL) {
    printf("Can't open output file.\n" );
    exit(2);
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
    exit(2);
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

  char* hist_file_name = getenv("HISTFILE");
  if (hist_file_name == NULL) {
    endwin();
    printf("Need to set HISTFILE env var\n");
    exit(2);
  }
  FILE* history_file = fopen(hist_file_name, "r");
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

void searchHistory(char* input, char* lines, bool* renderLine, int numLines, Match** matchesOut, int* nTokensOut) {
  // split into tokens
  int nTokens = 1;
  char* tokens = input;
  Match* matches = NULL;

  if (*input != '\0') {
    nTokens = 0;
    // figure out how many tokens there are
    int start = 0;
    bool foundChar = false;
    for (int i = 0; i < MAX_SEARCH_SIZE && input[i] != '\0'; i++) {
      if (!isWhitespace(input[i])) {
        foundChar = true;
      } else {
        if (i > 0 && foundChar && !isWhitespace(input[i-1]) && input[i-1] != '\\') {
          nTokens += 1;
          foundChar = false;
        }
      }
    }
    if (foundChar) {
      nTokens += 1;
    }

    tokens = malloc(nTokens*MAX_SEARCH_SIZE*sizeof(char));
    matches = malloc(numLines*nTokens*sizeof(Match));

    // copy token data into tokens array
    int tokenIndex = 0;
    start = 0;
    foundChar = false;
    int i;
    for (i = 0; i < MAX_SEARCH_SIZE && input[i] != '\0'; i++) {
      if (!isWhitespace(input[i])) {
        if (!foundChar) {
          foundChar = true;
          start = i;
        }
      } else {
        if (i > 0 && foundChar && !isWhitespace(input[i-1]) && input[i-1] != '\\') {
          int size = (i-start);
          memcpy(tokens+tokenIndex*MAX_SEARCH_SIZE, input+start, size*sizeof(char));
          tokens[tokenIndex*MAX_SEARCH_SIZE+size] = '\0';
          tokenIndex += 1;
          foundChar = false;
        }
      }
    }
    if (foundChar) {
      int size = (i-start);
      memcpy(tokens+tokenIndex*MAX_SEARCH_SIZE, input+start, size*sizeof(char));
      tokens[tokenIndex*MAX_SEARCH_SIZE+size] = '\0';
    }

    // fix any backslash spaces
    for (int i = 0; i < nTokens; i++) {
      char* loc = strstr(tokens+i*MAX_SEARCH_SIZE, "\\ ");
      while (loc != NULL) {
        memmove(loc, loc+1, (MAX_SEARCH_SIZE-(loc-(tokens+i*MAX_SEARCH_SIZE)))*sizeof(char));
        loc = strstr(tokens+i*MAX_SEARCH_SIZE, "\\ ");
      }
    }
  }

  StrMap* map = sm_new(50);

  for (int i = numLines-1; i >= 0; i--) {
    if (!sm_exists(map, lines+i*MAX_HISTORY_LINE_SIZE)) {
      int nMatches = 0;
      for (int j = 0; j < nTokens; j++) {
        char* start;
        if (*(tokens+j*MAX_SEARCH_SIZE) == '\0' || (start=strcasestr(lines+i*MAX_HISTORY_LINE_SIZE, tokens+j*MAX_SEARCH_SIZE)) != NULL) {
          if (matches != NULL) {
            matches[i*nTokens+j].start = (start-(lines+i*MAX_HISTORY_LINE_SIZE));
            matches[i*nTokens+j].size = strlen(tokens+j*MAX_SEARCH_SIZE);
          }
          nMatches++;
        } else {
          break;
        }
      }
      if (nMatches == nTokens) {
        sm_put(map, lines+i*MAX_HISTORY_LINE_SIZE, "");
        renderLine[i] = true;
      } else {
        renderLine[i] = false;
      }
    }
  }

  sm_delete(map);

  if (*input != '\0') {
    free(tokens);
  }

  *nTokensOut = nTokens;
  *matchesOut = matches;
}

void renderScreen(char* input, int cursor, int selected, char* lines, bool* renderLine, int numLines, Match* matches, int nTokens) {
  clear();

  int max_y;
  int max_x;
  getmaxyx(stdscr, max_y, max_x);

  mvprintw(0, 0, input);

  for (int i = 0; i < max_x; i++) {
    mvprintw(1, i, "-");
  }

  int line = 0;
  for (int i = numLines-1; i >= 0 && line < 50; i--) {
    mvprintw(line+2, 0, " ");
    if (renderLine[i]) {
      if (line == selected) {
        attron(COLOR_PAIR(1));
      }
      mvprintw(line+2, 1, lines+i*MAX_HISTORY_LINE_SIZE);
      if (matches != NULL) {
        attron(A_BOLD);
        for (int j = 0; j < nTokens; j++) {
          Match* match = matches+i*nTokens+j;
          char* tmp = malloc((match->size)*sizeof(char));
          memcpy(tmp, lines+i*MAX_HISTORY_LINE_SIZE+(match->start), match->size*sizeof(char));
          tmp[match->size] = '\0';
          mvprintw(line+2, match->start+1, tmp);
          free(tmp);
        }
        attroff(A_BOLD);
      }
      if (line == selected) {
        attroff(COLOR_PAIR(1));
      }
      line++;
    }
  }
  mvprintw(selected+2, 0, ">");

  move(0, cursor);
}

int main(int argc, char** argv) {
  int result = 10;

  // hack to get around ncurses blowing up stdout
  FILE* fd = fopen("/dev/tty", "r+");
  SCREEN* scr = newterm(NULL, fd, fd);
  setvbuf(stdout, NULL, _IONBF, 0);

  noecho();
  keypad(stdscr, TRUE);
  start_color();
  use_default_colors();
  init_pair(1, COLOR_RED, -1);

  char* input = malloc(MAX_SEARCH_SIZE*sizeof(char));
  input[0] = '\0';
  int size = 0;
  int cursor = 0;

  int numLines = 0;
  char* historyData = loadHistoryData(&numLines);

  bool* renderLine = malloc(numLines*sizeof(bool));
  memset(renderLine, 0, numLines*sizeof(bool));

  Match* matches = NULL;
  int nTokens = 0;

  int selected = 0;

  bool runCommand = false;

  searchHistory(input, historyData, renderLine, numLines, &matches, &nTokens);
  renderScreen(input, cursor, selected, historyData, renderLine, numLines, matches, nTokens);

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
        result = 10;
        break;
      case 0:
        runCommand = true;
        result = 11;
        break;
      default:
        if (c >= ' ' && c <= '~') {
          if (size < MAX_SEARCH_SIZE-1) {
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
      if (matches != NULL) {
        free(matches);
        matches = NULL;
      }
      selected = 0;
      searchHistory(input, historyData, renderLine, numLines, &matches, &nTokens);
    }

    renderScreen(input, cursor, selected, historyData, renderLine, numLines, matches, nTokens);
  }

  endwin();
  delscreen(scr);

  {
    int found = 0;
    for (int i = numLines-1; i >= 0; i--) {
      if (renderLine[i]) {
        found++;
        if (found > selected) {
          char* tmp = malloc(MAX_HISTORY_LINE_SIZE*sizeof(char));
          strcpy(tmp, historyData+i*MAX_HISTORY_LINE_SIZE);
          strtok(tmp, "\n");
          printf("%s\n", tmp);
          //outputCommand(argv[1], tmp);
          free(tmp);
          break;
        }
      }
    }
  }

  if (matches != NULL) {
    free(matches);
    matches = NULL;
  }
  free(historyData);
  free(input);
  free(renderLine);

  return result;
}
