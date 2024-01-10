include config.mk


# dispin/src dependencies
DEPS += -Isrc
#LIBS += -L. -l$(DISPIN)


# assume each .cpp file has main and build corresponding .exe executable
# - omit any .cpp files which should be built with separate recipes
SOURCES := $(filter-out stringSpinSim, $(basename $(wildcard *.cpp)))
EXES := $(addsuffix .exe, $(SOURCES))


#--------------------------------------------


all:
	$(MAKE) -C src
	make exe

exe: $(EXES)

stringSpinSim: DEPS += -I${PYTHIADIR}/include
stringSpinSim: LIBS += -L${PYTHIADIR}/lib -lpythia8 -ldl
stringSpinSim: DEPS += -I${STRINGSPINNER}
stringSpinSim: LIBS += ${STRINGSPINNER}/mc3P0.o ${STRINGSPINNER}/def.o -lgfortran
stringSpinSim: stringSpinSim.exe

%.exe: %.o
	@echo "--- make executable $@"
	$(CXX) -o $@ $< ./$(DISPINOBJ) $(LIBS)

%.o: %.cpp
	@echo "----- build $@ -----"
	$(CXX) -c $^ -o $@ $(FLAGS) $(DEPS)

clean:
	@cd src; make clean
	$(RM) $(EXES)
