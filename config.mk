# compiler and flags
CXX = g++
FLAGS = -g -Wno-deprecated -fPIC -fno-inline -Wno-write-strings
FLAGS += -DPARTICLE_BANK=$(PARTICLE_BANK)

# extra flags for valgrind
FLAGS += -O0

# ROOT
FLAGS += $(shell root-config --cflags)
DEPS = -I$(shell root-config --incdir)
LIBS = $(shell root-config --glibs)
LIBS += -lMinuit -lRooFitCore -lRooFit -lRooStats -lProof -lMathMore

# HIPO
$(if $(shell if pkg-config hipo4; then echo HIPO found; fi), , $(error HIPO not found))
DEPS += $(shell pkg-config hipo4 --cflags)
LIBS += $(shell pkg-config hipo4 --libs)

# Iguana
$(if $(shell if pkg-config iguana; then echo iguana found; fi), , $(error iguana not found))
DEPS += $(shell pkg-config iguana --cflags)
LIBS += $(shell pkg-config iguana --libs)

# BruFit
DEPS += -I$(BRUFIT)/core
LIBS += -L$(BRUFIT)/lib -lbrufit

# QADB
DEPS += -I$(QADB)/srcC/rapidjson/include -I$(QADB)/srcC/include

# StringSpinner + Pythia 8
ifdef STRINGSPINNER
	DEPS += -I${PYTHIADIR}/include
	LIBS += -L${PYTHIADIR}/lib -lpythia8 -ldl
	DEPS += -I${STRINGSPINNER}
	LIBS += ${STRINGSPINNER}/mc3P0.o ${STRINGSPINNER}/def.o -lgfortran
endif

# DiSpin shared object name and source directory
DISPIN = DiSpin
DISPINOBJ := lib$(DISPIN).so
