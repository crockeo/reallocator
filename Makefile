default:
	g++ -o reallocator -std=c++17 -O3 src/main.cpp

run: default
	./reallocator

clean:
	rm reallocator
