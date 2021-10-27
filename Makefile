# variable for execute tests 
EXECUTE_TEST = ON # OFF 

# variables to ambient 
FLAGS=-O2  -std=c++17 -Wall
CC:=c++ $(FLAGS)
SRC=src
TEST=$(SRC)/intel8080/i8080_tests
INCLUDE=$(SRC)/intel8080
OBJS=$(SRC)/intel8080/objs

# run install
all: install 

# compiler program and run tests
install:

	@ mkdir -p $(OBJS)
	@ $(CC) -c $(INCLUDE)/i8080.cpp -I $(INCLUDE)/ -o $(OBJS)/i8080.o
	@ $(CC) -c $(INCLUDE)/disassembly.cpp -I $(INCLUDE)/ -o $(OBJS)/disassembly.o 

	@ $(CC) -g $(TEST)/i8080_tests.cpp $(OBJS)/*.o -I $(INCLUDE)/ -o $(TEST)/bin/i8080_tests

	@ echo "*** compiled "

	@if [ $(EXECUTE_TEST) = ON ]; then          \
       $(TEST)/bin/i8080_tests; 			    \
	elif [ $(EXECUTE_TEST) != OFF ]; then       \
		echo "variable not set 'EXECUTE_TEST'\n \
		run tests set 'ON' to not run 'OFF'";   \
	else						                \
		echo "EXECUTE_TEST = $(EXECUTE_TEST)";  \
	fi


# remove files compileded
clean:
	@ rm -fr $(OBJS)  $(TEST)/bin/i8080_tests $(SRC)/intel8080/utils/disassembly.asm $(SRC)/intel8080/utils/memory.bin
run:
	@ $(TEST)/bin/i8080_tests