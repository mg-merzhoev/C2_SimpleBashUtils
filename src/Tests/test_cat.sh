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
    
    cat_output=$(eval "$2" 2>&1)
    s21_output=$(eval "$3" 2>&1)
    
    if diff <(echo "$cat_output") <(echo "$s21_output") > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}FAIL${NC}"
        FAILED=$((FAILED + 1))
    fi
}

echo -e "${YELLOW}=== S21_CAT INTEGRATION TESTS ===${NC}\n"

INPUT_FILE="test_cat.txt"

run_test "No flags, single file" \
    "cat $INPUT_FILE" \
    "../cat/s21_cat $INPUT_FILE"

run_test "No flags, multiple files" \
    "cat $INPUT_FILE $INPUT_FILE" \
    "../cat/s21_cat $INPUT_FILE $INPUT_FILE"

run_test "Flag -n (number all lines)" \
    "cat -n $INPUT_FILE" \
    "../cat/s21_cat -n $INPUT_FILE"

run_test "Flag -b (number non-blank lines)" \
    "cat -b $INPUT_FILE" \
    "../cat/s21_cat -b $INPUT_FILE"

run_test "Flag -s (squeeze blank lines)" \
    "cat -s $INPUT_FILE" \
    "../cat/s21_cat -s $INPUT_FILE"

run_test "Flag -e (show ends as \$)" \
    "cat -e $INPUT_FILE" \
    "../cat/s21_cat -e $INPUT_FILE"

run_test "Flag -E (show ends as \$, no -v)" \
    "cat -E $INPUT_FILE" \
    "../cat/s21_cat -E $INPUT_FILE"

run_test "Flag -t (show tabs as ^I)" \
    "cat -t $INPUT_FILE" \
    "../cat/s21_cat -t $INPUT_FILE"

run_test "Flag -T (show tabs as ^I, no -v)" \
    "cat -T $INPUT_FILE" \
    "../cat/s21_cat -T $INPUT_FILE"

run_test "Flag -v (show non-printing)" \
    "cat -v $INPUT_FILE" \
    "../cat/s21_cat -v $INPUT_FILE"

run_test "Combination -bn (b overrides n)" \
    "cat -bn $INPUT_FILE" \
    "../cat/s21_cat -bn $INPUT_FILE"

run_test "Combination -ne" \
    "cat -ne $INPUT_FILE" \
    "../cat/s21_cat -ne $INPUT_FILE"

run_test "Non-existent file" \
    "cat nonexistent.txt 2>&1 | head -1" \
    "../cat/s21_cat nonexistent.txt 2>&1 | head -1"

echo ""
echo -e "${YELLOW}=== RESULTS ===${NC}"
echo "Total:  $TOTAL"
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}ALL CAT TESTS PASSED!${NC}"
    exit 0
else
    echo -e "\n${RED}SOME CAT TESTS FAILED!${NC}"
    exit 1
fi