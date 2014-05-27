all:
	#g++ main.cc -I. -Wall -Wextra -std=c++11
	mpic++ src/main.cc -I./include/ -Wall -Wextra -std=c++11
