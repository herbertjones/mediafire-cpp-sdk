#!/bin/bash

SCRIPT_DIR=$(dirname "${BASH_SOURCE[0]}")
DEFAULT_TEST_NAME="ut_live_general"

if (( $# == 0 )); then
  TEST_NAME="$DEFAULT_TEST_NAME"
else
  TEST_NAME="$1"
fi

UNIT_TEST="${SCRIPT_DIR}/${TEST_NAME}"
NEW_ERRORS_FILE="${TEST_NAME}.log"
EXPECTED_ERRORS_FILE="${SCRIPT_DIR}/${TEST_NAME}-expected-failures.txt"

# Run unit test
"${UNIT_TEST}" -k "${NEW_ERRORS_FILE}"

# Process the error log
unexpected_errors=$(
( # Start subshell
# Skip the line that contains the number of tests run
read number_of_tests_line

# Count unexpected errors
sub_unexpected_errors="0"
while read error; do
  error_text=$(echo "${error}" | sed -E 's/[^\:]+:[0-9]+: (.*)/\1/g')
  if grep "^$error_text" "${EXPECTED_ERRORS_FILE}" 2>&1 >/dev/null; then
    # This error is expected
    # echo "Expected error: $error"
    :
  else
    (( ++sub_unexpected_errors ))
    echo "UNEXPECTED ERROR: ${error}" >&2
  fi
done
echo "${sub_unexpected_errors}"

) < "${NEW_ERRORS_FILE}" # End subshell
)

if (( unexpected_errors == 0 )); then
  echo "All failures are expected"
else
  echo "Encountered unexpected failures: ${unexpected_errors}"
fi

exit $unexpected_errors
