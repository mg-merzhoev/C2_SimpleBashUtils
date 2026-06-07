CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=201712L
TARGET = s21_grep
SRCS = s21_grep.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

rebuild: clean all

# Проверка стиля кода
style:
	clang-format -n $(SRCS)

# Автоматическое форматирование
format:
	clang-format -i $(SRCS)


# Запуск интеграционных тестов
test: $(TARGET)
	cd ../Tests && ./test_grep.sh


.PHONY: all clean rebuild style format