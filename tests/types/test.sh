
TEST_NAME=types

# clean
rm ${TEST_NAME}_gcc.txt
rm ${TEST_NAME}_bcc.txt
rm ${TEST_NAME}_gcc
rm ${TEST_NAME}_bcc

rm ${TEST_NAME}_bcc.asm
rm ${TEST_NAME}_bcc.o

rm *.*~
rm *.gv

# run the test


gcc -o ${TEST_NAME}_gcc ${TEST_NAME}.c
./${TEST_NAME}_gcc
echo $? > ${TEST_NAME}_gcc.txt

../../bcc -o ${TEST_NAME}_bcc ${TEST_NAME}.c
./${TEST_NAME}_bcc
echo $? > ${TEST_NAME}_bcc.txt

diff ${TEST_NAME}_gcc.txt ${TEST_NAME}_bcc.txt

if [ $? -eq 0 ]; then
    echo -e "\e[32mTest ${TEST_NAME} Passed\e[0m"
else
    echo -e "\e[31mTest ${TEST_NAME} Failed\e[0m"
fi


