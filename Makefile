include config.mk


# dispin/src dependencies
DEPS += -Isrc
#LIBS += -L. -l$(DISPIN)


# assume each .cpp file has main and build corresponding .exe executable
ifdef STRINGSPINNER
	SOURCES := $(basename $(wildcard *.cpp))
else
	SOURCES := $(filter-out stringSpinSim, $(basename $(wildcard *.cpp)))
endif
EXES := $(addsuffix .exe, $(SOURCES))

#--------------------------------------------

all:
	$(MAKE) -C src
	make exe

exe: $(EXES)

%.exe: %.o
	@echo "--- make executable $@"
	$(CXX) -o $@ $< ./$(DISPINOBJ) $(LIBS)

%.o: %.cpp
	@echo "----- build $@ -----"
	$(CXX) -c $^ -o $@ $(FLAGS) $(DEPS)

clean:
	@cd src; make clean
	$(RM) $(EXES)
