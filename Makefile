all: writetest

writetest: mkpath.h mkpath.cpp writeTest.cpp
	g++ mkpath.cpp writeTest.cpp -o writetest

clean: 
	rm *.write
	rm *.mmap
