DEBUGFLAGS    = -g
OPTFLAGS      = -O2 -std=c++11 -Wno-deprecated
ifeq (debug,$(findstring debug,$(LIMITBUILD)))
OPT           = $(DEBUGFLAGS)
NOOPT         =
else
OPT           = $(OPTFLAGS)
NOOPT         =
endif

# Compiler and flags -----------------------------------------------------------
CC = g++

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs --glibs)
ROOTINC = $(shell root-config --incdir)

CCFLAGS = -D STANDALONE $(ROOTCFLAGS) $(OPT) -Wall -fPIC
LIBS = $(ROOTLIBS) -l RooFit -lRooFitCore -l RooStats -l Minuit -l Foam -lMathMore \
       -lHistFactory -lXMLParser -lXMLIO -lNet -lSmatrix

# Library name -----------------------------------------------------------------
LIBNAME=AnaWSBuilder
SONAME=lib$(LIBNAME).so

# Linker and flags -------------------------------------------------------------
LD = g++
ROOTLDFLAGS   = $(shell root-config --ldflags)
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

dict: dirs 

obj: dict 

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc $(INC_DIR)/%.hh
	$(CC) $(CCFLAGS) -I $(INC_DIR) -c $< -o $@

$(OBJ_DIR)/$(DICTNAME).o :
	echo $(DICTHDRS)
	rootcling -f $(DICTNAME).cc -c -p -I$(INC_DIR) -I$(ROOTINC) $(DICTHDRS) $(INC_DIR)/LinkDef.h
	$(CC) $(CCFLAGS) -I $(INC_DIR) -c $(DICTNAME).cc -o $@
	rm -f $(DICTNAME).cc $(DICTNAME).h
#---------------------------------------

lib: dirs ${LIB_DIR}/$(SONAME)
${LIB_DIR}/$(SONAME):$(addprefix $(OBJ_DIR)/,$(OBJS))
		$(LD) $(LDFLAGS) $(addprefix $(OBJ_DIR)/,$(OBJS))  $(SOFLAGS) -o $@ $(LIBS)

#---------------------------------------

exe: $(addprefix $(EXE_DIR)/,$(EXES))
$(addprefix $(EXE_DIR)/,$(EXES)) : $(addprefix $(PROG_DIR)/,$(PROGS))
	$(CC) $< -o $@ $(CCFLAGS) -L $(LIB_DIR) -l $(LIBNAME) -I $(INC_DIR) $(LIBS)


#---------------------------------------

clean:
	@rm -rf $(DICTNAME).cc
	@rm -rf $(DICTNAME).hh
	@rm -rf $(OBJ_DIR) 
	@rm -rf $(EXE_DIR)
	@rm -rf $(LIB_DIR)
	@rm -rf $(DICTNAME)_rdict.pcm

#---------------------------------------
