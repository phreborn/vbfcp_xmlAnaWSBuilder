####export LIMITBUILD=debug when you want to debug#####
DEBUGFLAGS    = -g
#DEBUGFLAGS    = -gstabs
OPTFLAGS      = -O2 -std=c++11 -Wno-deprecated
# Optional compiler options for gcc >= 3.4.0
#OPTFLAGS      = -O3 -march=opteron
ifeq (debug,$(findstring debug,$(LIMITBUILD)))
OPT           = $(DEBUGFLAGS)
NOOPT         =
else
OPT           = $(OPTFLAGS)
NOOPT         =
endif



# Boost
# 64bit
ifeq ($(shell root-config --platform), macosx)
# please install boost on your machine
else
	BOOST_INC = -I/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/boost/boost-1.60.0-python2.7-x86_64-slc6-gcc49/boost-1.60.0-python2.7-x86_64-slc6-gcc49/include/
	BOOST_LIB = -L/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/boost/boost-1.60.0-python2.7-x86_64-slc6-gcc49/boost-1.60.0-python2.7-x86_64-slc6-gcc49/lib/
endif

# Compiler and flags -----------------------------------------------------------
CC = g++

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs --glibs)
ROOTINC = $(shell root-config --incdir)

CCFLAGS = -D STANDALONE $(ROOTCFLAGS) $(BOOST_INC) $(OPT) -Wall -fPIC
LIBS = $(ROOTLIBS) -l RooFit -lRooFitCore -l RooStats -l Minuit -l Foam -lMathMore \
       -lHistFactory -lXMLParser -lXMLIO -lNet -lSmatrix \
       -lboost_system -lboost_filesystem -lboost_program_options $(BOOST_LIB) \
 
# Library name -----------------------------------------------------------------
LIBNAME=AnaWSBuilder
SONAME=lib$(LIBNAME).so

# Linker and flags -------------------------------------------------------------
LD = g++
ROOTLDFLAGS   = $(shell root-config --ldflags)
#LDFLAGS       = $(ROOTLDFLAGS) $(OPT) -rdynamic -shared -Wl,-soname,$(SONAME) -fPIC 
LDFLAGS       = $(ROOTLDFLAGS) $(OPT) -rdynamic -shared -fPIC 

# Dictionaries filename --------------------------------------------------------
DICTNAME=cintdictionary

# Directory structure ----------------------------------------------------------
SRC_DIR = src
INC_DIR = inc
LIB_DIR = lib
PROG_DIR = bin
EXE_DIR = exe
OBJ_DIR = obj


# Useful shortcuts -------------------------------------------------------------
SRCS = $(notdir $(shell ls $(SRC_DIR)/*.cc|grep -v $(DICTNAME) ))
SRCS += $(DICTNAME).cc
OBJS = $(SRCS:.cc=.o)
PROGS = $(notdir $(wildcard ${PROG_DIR}/*.cpp)) 
EXES = $(PROGS:.cpp=)

# Classes with dicts -----------------------------------------------------------
DICTHDRS= $(notdir $(shell grep -l ClassDef ${INC_DIR}/*hh))
#Makefile Rules ---------------------------------------------------------------
.PHONY: clean dirs dict obj lib exe

all: dirs dict obj lib exe

#---------------------------------------

dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(EXE_DIR)

#---------------------------------------

dict: dirs #$(SRC_DIR)/$(DICTNAME).cc

# $(SRC_DIR)/$(DICTNAME).cc : $(INC_DIR)/LinkDef.h
# 	@echo "\n*** Generating dictionaries ..."
# 	mv $(SRC_DIR)/$(DICTNAME).h $(INC_DIR)/$(DICTNAME).h
#---------------------------------------

obj: dict 

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc $(INC_DIR)/%.hh
	$(CC) $(CCFLAGS) -I $(INC_DIR) -c $< -o $@

$(OBJ_DIR)/$(DICTNAME).o : #$(DICTHDRS)
	echo $(DICTHDRS)
	rootcint -f $(DICTNAME).cc -c -p -I$(INC_DIR) -I$(ROOTINC) $(DICTHDRS) $(INC_DIR)/LinkDef.h
	$(CC) $(CCFLAGS) -I $(INC_DIR) -c $(DICTNAME).cc -o $@
	rm -f $(DICTNAME).cc $(DICTNAME).h
#---------------------------------------

lib: dirs ${LIB_DIR}/$(SONAME)
${LIB_DIR}/$(SONAME):$(addprefix $(OBJ_DIR)/,$(OBJS))
# 		@echo "\n*** Building $(SONAME) library:"
		$(LD) $(LDFLAGS) $(BOOST_INC) $(addprefix $(OBJ_DIR)/,$(OBJS))  $(SOFLAGS) -o $@ $(LIBS)

#---------------------------------------

exe: $(addprefix $(EXE_DIR)/,$(EXES))
# 	@echo "\n*** Compiling executables ..."
$(addprefix $(EXE_DIR)/,$(EXES)) : $(addprefix $(PROG_DIR)/,$(PROGS))
	$(CC) $< -o $@ $(CCFLAGS) -L $(LIB_DIR) -l $(LIBNAME) -I $(INC_DIR) $(BOOST_INC) $(LIBS)


#---------------------------------------

clean:
# 	@echo "*** Cleaning all directories and dictionaries ..."
	@rm -rf $(DICTNAME).cc
	@rm -rf $(DICTNAME).hh
	@rm -rf $(OBJ_DIR) 
	@rm -rf $(EXE_DIR)
	@rm -rf $(LIB_DIR)

#---------------------------------------
