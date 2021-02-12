#########################################################################################################
# Package: xmlAnaWSBuilder ##############################################################################
cmake_minimum_required( VERSION 2.8 )

# Declare the package name:
project( xmlAnaWSBuilder )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "..." FORCE)
endif()
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)

file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/exe)

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

# require ROOT
find_package( ROOT REQUIRED COMPONENTS RIO RooFitCore RooFit HistFactory RooStats MathCore MathMore Physics Minuit Foam XMLParser XMLIO )
include(${ROOT_USE_FILE})

# require RooFitExtensions
list(APPEND CMAKE_PREFIX_PATH $ENV{RooFitExtensions_DIR})
find_package(RooFitExtensions REQUIRED CONFIG)
include_directories(${RooFitExtensions_INCLUDE_DIRS})
link_directories(${RooFitExtensions_LIBRARY_DIRS})

set(xmlAnaWSBuilderHeaders "")
set(xmlAnaWSBuilderSources "")

file(GLOB incPaths inc/*)
file(GLOB srcPaths src/*)

foreach (_incPath ${incPaths})
	get_filename_component(_incName ${_incPath} NAME)
    	if(NOT ${_incName} MATCHES "LinkDef.h")
    	       list(APPEND xmlAnaWSBuilderHeaders ${_incName})
	endif()
endforeach()

foreach (_srcPath ${srcPaths})
	get_filename_component(_srcName ${_srcPath} NAME)
    	if(NOT ${_srcName} MATCHES "cintdictionary.cc")
    	       list(APPEND xmlAnaWSBuilderSources "src/${_srcName}")
	endif()
endforeach()

include_directories(${PROJECT_SOURCE_DIR}/inc)

# generate the dictionary source code
ROOT_GENERATE_DICTIONARY(G__xmlAnaWSBuilder ${xmlAnaWSBuilderHeaders} LINKDEF inc/LinkDef.h)

add_library( xmlAnaWSBuilder SHARED ${xmlAnaWSBuilderSources} G__xmlAnaWSBuilder.cxx)
target_link_libraries( xmlAnaWSBuilder ${ROOT_LIBRARIES} ${RooFitExtensions_LIBRARIES} )

#add executables
add_executable( XMLReader app/XMLReader.cpp )
target_link_libraries( XMLReader xmlAnaWSBuilder )

# Installation
install(TARGETS XMLReader DESTINATION bin)
install(TARGETS xmlAnaWSBuilder LIBRARY DESTINATION lib)
