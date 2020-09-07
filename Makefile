default:
	g++ -o reallocator -fsanitize=address -std=c++17 src/main.cpp

run: default
	./reallocator

clean:
	rm reallocator
