#!/bin/sh

run_test() {
  echo "$1"
  sleep 1
  i=1
  while [ "$i" -le 1000 ]; do
    ./"$1"
    i=$((i + 1))
  done
}

for test_name in test2 test2b; do
  run_test "$test_name"
done