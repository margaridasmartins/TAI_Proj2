all: lang findlang locatelang

lang: lang.cpp
	g++ -Wall -g -o ../bin/lang lang.cpp fcm.hpp lang.hpp -lm

findlang: findlang.cpp
	g++ -Wall -g -o ../bin/findlang findlang.cpp fcm.hpp lang.hpp -lm

locatelang: locatelang.cpp
	g++ -Wall -g -o ../bin/locatelang locatelang.cpp fcm.hpp lang.hpp -lm