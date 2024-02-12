# specify which particle bank to read (data vs. MC)
# 0 = REC::Particle -- for data
# 1 = MC::Lund -- for MC generated (helicity injected)
# 2 = MC::Particle -- for MC generated (helicity injected)
# 3 = REC::Particle -- for MC reconstructed (helicity set by event-match to MCgen)
PARTICLE_BANK = 0


####################################

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

