record: 
	g++ record.cpp -o record -O2 -Ieigen-3.4.0 -lm -std=c++17

clean:
	rm -rf record
