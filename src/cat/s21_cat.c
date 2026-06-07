#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int b_flag; /* нумерует только непустые строки */
  int e_flag; /* отображает символы конца строки как $ */
  int n_flag; /* нумерует все выходные строки */
  int s_flag; /* сжимает несколько смежных пустых строк */
  int t_flag; /* отображает табы как ^I */
  int v_flag; /* отображает непечатаемые символы */
} options;

/* Инициализация опций */
void init_options(options *opt) {
  opt->b_flag = 0;
  opt->e_flag = 0;
  opt->n_flag = 0;
  opt->s_flag = 0;
  opt->t_flag = 0;
  opt->v_flag = 0;
}

/* Отображение непечатаемых символов */
void print_visible_char(int ch, options *opt) {
  if (opt->v_flag) {
    if (ch == 127) {
      printf("^?");
    } else if (ch < 32 && ch != 9 && ch != 10) {
      printf("^%c", ch + 64);
    } else if (ch > 127 && ch < 160) {
      printf("M-^%c", ch - 128 + 64);
    } else {
      putchar(ch);
    }
  } else {
    putchar(ch);
  }
}

/* Отображение табуляции */
void print_tab(options *opt) {
  if (opt->t_flag) {
    printf("^I");
  } else {
    putchar('\t');
  }
}

/* Проверка, является ли строка пустой */
int is_empty_line(const char *line, int len) {
  for (int i = 0; i < len; i++) {
    if (line[i] != '\n' && line[i] != '\r' && line[i] != ' ' &&
        line[i] != '\t') {
      return 0;
    }
  }
  return 1;
}

/* Обработка одной строки с учётом флага -s */
int should_squeeze_line(const char *line, int len, int prev_empty,
                        options *opt) {
  int current_empty = is_empty_line(line, len);
  if (opt->s_flag && current_empty && prev_empty) {
    return 1; /* строку нужно пропустить */
  }
  return 0; /* строку нужно вывести */
}

/* Обработка одного файла */
void process_file(const char *filename, options *opt) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "cat: %s: No such file or directory\n", filename);
    return;
  }

  char line[8192];
  int line_number = 1;
  int prev_empty = 0;

  while (fgets(line, sizeof(line), file) != NULL) {
    int line_len = strlen(line);

    /* Проверка на сжатие пустых строк */
    int skip = should_squeeze_line(line, line_len, prev_empty, opt);

    if (!skip) {
      /* Определяем, нужно ли нумеровать */
      int need_number = 0;
      if (opt->n_flag) {
        need_number = 1;
      } else if (opt->b_flag && !is_empty_line(line, line_len - 1)) {
        need_number = 1;
      }

      /* Вывод номера строки */
      if (need_number) {
        printf("%6d\t", line_number);
      }

      /* Вывод содержимого строки */
      for (int i = 0; i < line_len; i++) {
        char ch = line[i];
        if (ch == '\t') {
          print_tab(opt);
        } else if (ch == '\n') {
          if (opt->e_flag) {
            printf("$\n");
          } else {
            putchar('\n');
          }
        } else {
          print_visible_char(ch, opt);
        }
      }

      /* Увеличиваем номер строки */
      if (need_number) {
        line_number++;
      } else if (opt->n_flag) {
        line_number++;
      }
    }

    /* Обновляем состояние для сжатия */
    prev_empty = is_empty_line(line, line_len - 1);
  }

  fclose(file);
}

/* Разбор аргументов командной строки */
int parse_arguments(int argc, char **argv, options *opt) {
  int opt_char;

  while ((opt_char = getopt(argc, argv, "benstTvE")) != -1) {
    switch (opt_char) {
      case 'b':
        opt->b_flag = 1;
        break;
      case 'e':
        opt->e_flag = 1;
        opt->v_flag = 1;
        break;
      case 'E':
        opt->e_flag = 1;
        break;
      case 'n':
        opt->n_flag = 1;
        break;
      case 's':
        opt->s_flag = 1;
        break;
      case 't':
        opt->t_flag = 1;
        opt->v_flag = 1;
        break;
      case 'T':
        opt->t_flag = 1;
        break;
      case 'v':
        opt->v_flag = 1;
        break;
      default:
        return 0;
    }
  }

  return 1;
}

int main(int argc, char **argv) {
  options opt;
  init_options(&opt);

  if (!parse_arguments(argc, argv, &opt)) {
    fprintf(stderr,
            "Usage: %s [-b] [-e] [-E] [-n] [-s] [-t] [-T] [-v] [file...]\n",
            argv[0]);
    return 1;
  }

  /* Обработка конфликтующих флагов */
  if (opt.b_flag && opt.n_flag) {
    opt.n_flag = 0;
  }

  if (optind >= argc) {
    fprintf(stderr,
            "Usage: %s [-b] [-e] [-E] [-n] [-s] [-t] [-T] [-v] [file...]\n",
            argv[0]);
    return 1;
  }

  for (int i = optind; i < argc; i++) {
    process_file(argv[i], &opt);
  }

  return 0;
}