#!/bin/bash

function validate() {
	expected="$(cat test/build/$1.output)"
	actual="$(./build/ast-interpreter "$(cat test/$1.c)" 2>&1)"

	if [ $expected == $actual ]; then
	echo 'success'
	else
	echo 'error'
	echo 'expected: ' $expected
	echo 'actual: ' $actual
	exit -1
	fi
}

test_cases=$(find test/build -name "*.output")
for test_case in $test_cases
do
	c=${test_case/'test/build/'/}
	c=${c%%.*}
	echo $c
	validate $c
done
