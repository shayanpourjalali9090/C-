all: src/*.cpp
	mkdir -p bin
	g++ -Iinclude src/*.cpp -o bin/main