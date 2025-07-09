find ./tests/ -name test.sh -exec bash -c 'cd "$(dirname "$0")" && ./test.sh' {} \;
