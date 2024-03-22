#PA2 cpp compiler
.PHONY: all run clean
all: 108501006_PA2.o
	@g++ -std=c++11 108501006_PA2.o -o exe
run:
	@./exe $(Testcase)
clean:
	@rm *.o
	@rm exe
108501006_PA2.o: 108501006_PA2.cpp
	@g++ -std=c++11 -c 108501006_PA2.cpp

