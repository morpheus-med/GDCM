declare RED="\e[31m"
declare GREEN="\e[32m"
declare CYAN="\e[36m"
declare DEFAULT="\e[39m"

declare passed=0
declare failed=0

function compare_val() {
    declare title="$1"
    declare actual="$2"
    declare expected="$3"
    echo -e "${CYAN}${title}${DEFAULT}"
    if [[ $actual = $expected ]]; then
        echo -e "${GREEN}PASSED${DEFAULT}"
        passed=$((passed+1))
        return 0
    else
        echo "actual: ${actual}"
        echo "expect: ${expected}"
        echo -e "${RED}FAILED${DEFAULT}"
        failed=$((failed+1))
        return 1
    fi
}

function verify_results() {
    declare result="TOTAL PASSED: ${passed} FAILED: ${failed}"
    if [[ ${failed} -gt 0 ]]; then
      echo -e "${RED}${result}${DEFAULT}"
    else
      echo -e "${GREEN}${result}${DEFAULT}"
    fi
    exit ${failed}
}
