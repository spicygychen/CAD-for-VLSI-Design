#PA3 cpp compiler
.PHONY: all run clean
all: 108501006_PA3.o
	@g++ -std=c++11 108501006_PA3.o -o exe
run:
	@./exe $(input) $(output)
clean:
	@rm *.o
	@rm exe
108501006_PA3.o: 108501006_PA3.cpp
	@g++ -std=c++11 -c 108501006_PA3.cpp
