CPPFLAGS=-Isrc/ -std=c++17 -O3 -s -DNDEBUG -Werror -pthread

default: build

build: src/main.cpp
	mkdir compiled
	g++ $(CPPFLAGS) -o compiled/httpd src/main.cpp
