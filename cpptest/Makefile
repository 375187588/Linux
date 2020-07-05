object:hello.cpp Bird.h Animation.h
	g++ -o object hello.cpp Bird.h Animation.h -std=c++11

cc=g++
prom =object
deps=$(shell find ./ -name "*.h")
src=$(shell find ./ -name "*.cpp")
obj=$(src:%.c=%.o)

$(prom):$(obj)
	$(cc) -o $(prom) $(obj)

%.o:%.cpp $(deps)
	$(cc) -c $< -o $@

clean:
	rm -rf $(obj) $(prom)

