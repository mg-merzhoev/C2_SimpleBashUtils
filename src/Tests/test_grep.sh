#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

TOTAL=0
PASSED=0
FAILED=0

run_test() {
    TOTAL=$((TOTAL + 1))
    echo -n "Test $TOTAL: $1 ... "
    
    grep_output=$(eval "$2" 2>&1)
    s21_output=$(eval "$3" 2>&1)
    
    if diff <(echo "$grep_output") <(echo "$s21_output") > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}FAIL${NC}"
        FAILED=$((FAILED + 1))
    fi
}

echo -e "${YELLOW}=== S21_GREP INTEGRATION TESTS ===${NC}\n"

FILE1="test_grep1.txt"
FILE2="test_grep2.txt"

# Базовые тесты
run_test "No flags, single file" \
    "grep 'hello' $FILE1" \
    "../grep/s21_grep 'hello' $FILE1"

run_test "No flags, multiple files" \
    "grep 'hello' $FILE1 $FILE2" \
    "../grep/s21_grep 'hello' $FILE1 $FILE2"

# Флаги
run_test "Flag -i (ignore case)" \
    "grep -i 'hello' $FILE1" \
    "../grep/s21_grep -i 'hello' $FILE1"

run_test "Flag -v (invert match)" \
    "grep -v 'hello' $FILE1" \
    "../grep/s21_grep -v 'hello' $FILE1"

run_test "Flag -n (line numbers)" \
    "grep -n 'hello' $FILE1" \
    "../grep/s21_grep -n 'hello' $FILE1"

run_test "Flag -c (count matches)" \
    "grep -c 'hello' $FILE1" \
    "../grep/s21_grep -c 'hello' $FILE1"

run_test "Flag -l (files with match)" \
    "grep -l 'hello' $FILE1 $FILE2" \
    "../grep/s21_grep -l 'hello' $FILE1 $FILE2"

run_test "Flag -e (multiple patterns)" \
    "grep -e 'hello' -e 'world' $FILE1" \
    "../grep/s21_grep -e 'hello' -e 'world' $FILE1"

# Комбинации флагов
run_test "Combination -iv" \
    "grep -iv 'hello' $FILE1" \
    "../grep/s21_grep -iv 'hello' $FILE1"

run_test "Combination -in" \
    "grep -in 'hello' $FILE1" \
    "../grep/s21_grep -in 'hello' $FILE1"

run_test "Combination -ic" \
    "grep -ic 'hello' $FILE1" \
    "../grep/s21_grep -ic 'hello' $FILE1"

run_test "Combination -vc" \
    "grep -vc 'hello' $FILE1" \
    "../grep/s21_grep -vc 'hello' $FILE1"

run_test "Combination -nv" \
    "grep -nv 'hello' $FILE1" \
    "../grep/s21_grep -nv 'hello' $FILE1"

run_test "Combination -e -i" \
    "grep -e 'hello' -e 'world' -i $FILE1" \
    "../grep/s21_grep -e 'hello' -e 'world' -i $FILE1"

run_test "Combination -e -n" \
    "grep -e 'hello' -e 'world' -n $FILE1" \
    "../grep/s21_grep -e 'hello' -e 'world' -n $FILE1"

# POSIX классы
run_test "POSIX class digits" \
    "grep '[[:digit:]]' $FILE1" \
    "../grep/s21_grep '[[:digit:]]' $FILE1"

run_test "POSIX class alpha" \
    "grep '[[:alpha:]]' $FILE1" \
    "../grep/s21_grep '[[:alpha:]]' $FILE1"

# Регулярные выражения
run_test "Regex: line start ^" \
    "grep '^start' $FILE1" \
    "../grep/s21_grep '^start' $FILE1"

run_test "Regex: line end $" \
    "grep 'here$' $FILE1" \
    "../grep/s21_grep 'here$' $FILE1"

# Граничные случаи
run_test "Non-existent file" \
    "grep 'hello' nonexistent.txt 2>&1 | head -1" \
    "../grep/s21_grep 'hello' nonexistent.txt 2>&1 | head -1"


echo ""
echo -e "${YELLOW}=== RESULTS ===${NC}"
echo "Total:  $TOTAL"
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}ALL GREP TESTS PASSED!${NC}"
    exit 0
else
    echo -e "\n${RED}SOME GREP TESTS FAILED!${NC}"
    exit 1
fi