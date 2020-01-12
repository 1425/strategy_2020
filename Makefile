CXXFLAGS+=-Wall -Wextra -Werror -std=c++1z -Ofast -flto -march=native -mtune=native

test: strategy
	time ./strategy --team 2910 #--file data/2019wamou.csv

strategy: util.o input_data.o 

.PHONY: clean
clean:
	rm -f *.o
	rm -f strategy 
