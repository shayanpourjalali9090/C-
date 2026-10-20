all: src/*.cpp
	mkdir -p bin
	g++ -Wall -Iinclude src/*.cpp -o bin/main