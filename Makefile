EXECUTABLE=reallocator

default:
	clang++ -o ${EXECUTABLE} -std=c++17 -O3 src/main.cpp

profile:
	clang++ -o reallocator -std=c++17 -lprofiler -g src/main.cpp
	CPUPROFILE=cpu_prof ./${EXECUTABLE}
	pprof --text ${EXECUTABLE} cpu_prof

run: default
	./${EXECUTABLE}

clean:
	rm ${EXECUTABLE}
