CPPFLAGS=-Isrc/ -std=c++17 -O3 -s -DNDEBUG -Werror -pthread
#CPPFLAGS=-Isrc/ -std=c++17 -Werror -pthread

default: build

build: src/main.cpp
	mkdir -p compiled
	g++ $(CPPFLAGS) -o compiled/httpd src/main.cpp
