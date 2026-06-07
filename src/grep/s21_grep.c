#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int i_flag;
  int v_flag;
  int c_flag;
  int l_flag;
  int n_flag;
  char **patterns;
  int pattern_count;
} options;

/* Добавление шаблона в массив */
void add_pattern(options *opt, const char *pattern) {
  opt->pattern_count++;
  opt->patterns = realloc(opt->patterns, opt->pattern_count * sizeof(char *));
  opt->patterns[opt->pattern_count - 1] = strdup(pattern);
}

/* Компиляция всех регулярных выражений */
int compile_regexes(regex_t *regexes, options *opt, int flags) {
  for (int i = 0; i < opt->pattern_count; i++) {
    if (regcomp(&regexes[i], opt->patterns[i], flags) != 0) {
      return 1;
    }
  }
  return 0;
}

/* Освобождение регулярных выражений */
void free_regexes(regex_t *regexes, int count) {
  for (int i = 0; i < count; i++) {
    regfree(&regexes[i]);
  }
}

/* Проверка совпадения строки с любым шаблоном */
int check_match(regex_t *regexes, int pattern_count, const char *line) {
  for (int i = 0; i < pattern_count; i++) {
    if (regexec(&regexes[i], line, 0, NULL, 0) == 0) {
      return 1;
    }
  }
  return 0;
}

/* Вывод строки с учётом флагов */
void print_line(const char *filename, int line_number, const char *line,
                int multiple_files, options *opt) {
  if (opt->l_flag) {
    return;
  }
  if (multiple_files) {
    printf("%s:", filename);
  }
  if (opt->n_flag) {
    printf("%d:", line_number);
  }
  printf("%s\n", line);
}

/* Вывод количества совпадений */
void print_count(const char *filename, int count, int multiple_files,
                 options *opt) {
  if (opt->l_flag) {
    return;
  }
  if (multiple_files) {
    printf("%s:", filename);
  }
  printf("%d\n", count);
}

/* Обработка одного файла */
void process_file(const char *filename, options *opt, int multiple_files) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "grep: %s: No such file or directory\n", filename);
    return;
  }

  int flags = REG_EXTENDED;
  if (opt->i_flag) {
    flags |= REG_ICASE;
  }

  regex_t regexes[opt->pattern_count];

  if (compile_regexes(regexes, opt, flags)) {
    fprintf(stderr, "grep: Invalid regular expression\n");
    fclose(file);
    return;
  }

  char line[8192];
  int line_number = 1;
  int match_count = 0;
  int printed_filename = 0;

  while (fgets(line, sizeof(line), file)) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }

    int matched = check_match(regexes, opt->pattern_count, line);
    if (opt->v_flag) {
      matched = !matched;
    }

    if (matched) {
      match_count++;
      if (opt->l_flag && !printed_filename) {
        printf("%s\n", filename);
        printed_filename = 1;
        break;
      }
      if (!opt->c_flag && !opt->l_flag) {
        print_line(filename, line_number, line, multiple_files, opt);
      }
    }
    line_number++;
  }

  if (opt->c_flag && !opt->l_flag) {
    print_count(filename, match_count, multiple_files, opt);
  }

  free_regexes(regexes, opt->pattern_count);
  fclose(file);
}

/* Разбор аргументов командной строки */
int parse_arguments(int argc, char **argv, options *opt) {
  int opt_char;
  opt->i_flag = 0;
  opt->v_flag = 0;
  opt->c_flag = 0;
  opt->l_flag = 0;
  opt->n_flag = 0;
  opt->patterns = NULL;
  opt->pattern_count = 0;

  while ((opt_char = getopt(argc, argv, "e:ivcln")) != -1) {
    switch (opt_char) {
      case 'e':
        add_pattern(opt, optarg);
        break;
      case 'i':
        opt->i_flag = 1;
        break;
      case 'v':
        opt->v_flag = 1;
        break;
      case 'c':
        opt->c_flag = 1;
        break;
      case 'l':
        opt->l_flag = 1;
        break;
      case 'n':
        opt->n_flag = 1;
        break;
      default:
        return 0;
    }
  }
  return 1;
}

/* Получение шаблона из аргументов (если не было -e) */
void get_pattern_from_args(int argc, char **argv, options *opt) {
  if (opt->pattern_count == 0 && optind < argc) {
    add_pattern(opt, argv[optind]);
    optind++;
  }
}

/* Проверка валидности аргументов */
int validate_args(options *opt, int argc) {
  if (opt->pattern_count == 0) {
    return 0;
  }
  if (optind >= argc) {
    return 0;
  }
  return 1;
}

/* Освобождение памяти шаблонов */
void free_patterns(options *opt) {
  for (int i = 0; i < opt->pattern_count; i++) {
    free(opt->patterns[i]);
  }
  free(opt->patterns);
}

int main(int argc, char **argv) {
  options opt;

  /* Проверка: нет аргументов вообще */
  if (argc == 1) {
    fprintf(stderr,
            "Usage: grep [-e pattern] [-i] [-v] [-c] [-l] [-n] [pattern] "
            "[file...]\n");
    return 1;
  }

  if (!parse_arguments(argc, argv, &opt)) {
    fprintf(stderr,
            "Usage: grep [-e pattern] [-i] [-v] [-c] [-l] [-n] [pattern] "
            "[file...]\n");
    return 1;
  }

  /* Проверка: нет ни одного аргумента после флагов и нет шаблонов */
  if (optind >= argc && opt.pattern_count == 0) {
    fprintf(stderr,
            "Usage: grep [-e pattern] [-i] [-v] [-c] [-l] [-n] [pattern] "
            "[file...]\n");
    return 1;
  }

  get_pattern_from_args(argc, argv, &opt);

  if (!validate_args(&opt, argc)) {
    fprintf(stderr,
            "Usage: grep [-e pattern] [-i] [-v] [-c] [-l] [-n] [pattern] "
            "[file...]\n");
    free_patterns(&opt);
    return 1;
  }

  int multiple_files = (argc - optind > 1);

  for (int i = optind; i < argc; i++) {
    process_file(argv[i], &opt, multiple_files);
  }

  free_patterns(&opt);
  return 0;
}