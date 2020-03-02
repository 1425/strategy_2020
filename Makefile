CXXFLAGS+=-Wall -Wextra -Werror -std=c++1z -Ofast -flto -march=native -mtune=native

test: strategy
	time ./strategy --team 2910 #--file data/2019wamou.csv

strategy: util.o input_data.o capabilities.o dist.o game.o

in_match: input_data.o util.o

preview: util.o input_data.o dist.o capabilities.o game.o

viper_preview: util.o dist.o capabilities.o game.o viper.o

handheld: util.o

r5:
	make handheld 2>&1
	./handheld

from_tba: ../tba/db.o ../tba/data.o ../tba/rapidjson.o ../tba/curl.o ../tba/util.o from_tba.o util.o capabilities.o dist.o
	$(CXX) $(CXXFLAGS) $^ -lsqlite3 -lcurl -o $@

from_tba_test:
	make from_tba 2>&1
	./from_tba

viper_pick: game.o viper.o dist.o capabilities.o util.o

viper_test:
	make viper_pick 2>&1
	./viper_pick --path viper.txt

plots: ../tba/db.o plots.o ../tba/data.o ../tba/rapidjson.o ../tba/curl.o util.o
	$(CXX) $(CXXFLAGS) $^ -lsqlite3 -lcurl -o $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f strategy 
