# specify which particle bank to read (data vs. MC)
# 0 = REC::Particle -- for data
# 1 = MC::Lund -- for MC generated (helicity injected)
# 2 = MC::Particle -- for MC generated (helicity injected)
# 3 = REC::Particle -- for MC reconstructed (helicity set by event-match to MCgen)
PARTICLE_BANK = 0


####################################

# compiler and flags
CXX = g++ -std=c++11
FLAGS = -g -Wno-deprecated -fPIC -m64 -fno-inline -Wno-write-strings
FLAGS += -DPARTICLE_BANK=$(PARTICLE_BANK)

# extra flags for valgrind
FLAGS += -O0


# ROOT
DEPS = $(shell root-config --cflags)
LIBS = $(shell root-config --glibs)
LIBS += -lMinuit -lRooFitCore -lRooFit -lRooStats

# HIPO
#DEPS += -I$(HIPO)/hipo4
#LIBS += -L$(HIPO)/lib -lhipo4

# clas12root
#DEPS += -I$(CLAS12ROOT)/hipo4 -I$(CLAS12ROOT)/Clas12Banks -I$(CLAS12ROOT)/Clas12Root
#LIBS += -L$(CLAS12ROOT)/lib -lClas12Banks -lClas12Root -lHipo4 -llz4

# QADB
DEPS += -I$(QADB)/srcC/rapidjson/include -I$(QADB)/srcC/include

# DiSpin shared object name and source directory
DISPIN = DiSpin
DISPINOBJ := lib$(DISPIN).so

