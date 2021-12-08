all: lang findlang

lang: lang.cpp
	g++ -Wall -g -o ../bin/lang lang.cpp fcm.hpp lang.hpp -lm

findlang: findlang.cpp
	g++ -Wall -g -o ../bin/findlang findlang.cpp fcm.hpp lang.hpp -lm
