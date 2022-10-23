expected="$(cat test/$1.output)"
actual="$(./build/ast-interpreter "$(cat test/$1.c)" 2>&1)"

if [ $expected == $actual ]; then
 echo 'success'
else
 echo 'error'
 echo 'expected: ' $expected
 echo 'actual: ' $actual
fi
