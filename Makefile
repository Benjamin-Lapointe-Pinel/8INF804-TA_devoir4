MAKEFLAGS+=-j
NAME=a.out
SOURCES=$(wildcard sources/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
DEPENDS=$(OBJECTS:.o=.d)
INCLUDES=-I. -Iincludes/
LIBRARIES=-lboost_program_options -lstdc++fs
CXXFLAGS=$(INCLUDES) $(LIBRARIES) -W -Wall -Wextra -Wno-unused-function

.PHONY: release debug build rebuild tags lint clean

debug: CXXFLAGS+=-g
debug: build tags

release: CXXFLAGS+=-O3
release: rebuild

build: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $? -o $(NAME)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

rebuild:
	$(MAKE) clean
	$(MAKE) build

# Verify if programs are present:
# if which dot 1>/dev/null; then echo allo; fi (TODO also redirect stderr)
tags:
	ctags -R

lint: $(SOURCES)
	clang-tidy -header-filter=.* $? -- $(CXXFLAGS) $(INCLUDES)

clean:
	rm -f *.d $(DEPENDS)
	rm -f *.o $(OBJECTS)
	rm -f *.out $(NAME)
	rm -f tags

-include $(DEPENDS)
