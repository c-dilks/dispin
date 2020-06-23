include ../config.mk

# rootcling dictionary
DICT := $(DISPIN)Dict.cxx
PCM := $(DISPIN)Dict_rdict.pcm
LINKDEF = LinkDef.h


# source code (with $(DICT) and $(LINKDEF) moved to end of lists for rootcling)
SOURCES := $(filter-out $(DICT), $(wildcard *.cxx)) $(DICT)
HEADERS := $(filter-out $(LINKDEF), $(wildcard *.h)) $(LINKDEF)


#-----------------------------------------------


$(DISPINOBJ): $(DICT) $(HEADERS) $(SOURCES)
	@echo "----- build $@ -----"
	$(CXX) $(SOURCES) -shared -o ../$@ $(FLAGS) $(DEPS) $(LIBS)

$(DICT): $(HEADERS)
	@echo "----- generate dictionary $@ -----"
	@rootcling -f $@ -c $^

clean:
	$(RM) ../$(DISPINOBJ) $(DICT) $(PCM)
