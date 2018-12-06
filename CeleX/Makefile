#############################################################################
# Makefile for building: libCeleX.so
#############################################################################

MAKEFILE      = Makefile

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DCELEX_LIBRARY
CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O2 -std=gnu++0x -Wall -W -D_REENTRANT -fPIC $(DEFINES)
INCPATH       = -I../CeleX -I.
DEL_FILE      = rm -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p
COPY          = cp -f
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
INSTALL_FILE  = install -m 644 -p
INSTALL_PROGRAM = install -m 755 -p
INSTALL_DIR   = cp -f -R
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
TAR           = tar -cf
COMPRESS      = gzip -9f
LINK          = g++
LFLAGS        = -Wl,-O1 -Wl,-rpath, -shared -Wl,-soname,libCeleX.so
LIBS          = $(SUBLIBS) -lokFrontPanel -lCeleDriver
AR            = ar cqs
RANLIB        = 
SED           = sed
STRIP         = strip

####### Output directory

OBJECTS_DIR   = ./

####### Files


SOURCES       = ../CeleX/base/xbase.cpp \
		../CeleX/base/dataqueue.cpp \
		../CeleX/configproc/tinyxml/tinyxmlparser.cpp \
		../CeleX/configproc/tinyxml/tinyxmlerror.cpp \
		../CeleX/configproc/tinyxml/tinyxml.cpp \
		../CeleX/configproc/tinyxml/tinystr.cpp \
		../CeleX/eventproc/celex5.cpp \
		../CeleX/eventproc/celex4.cpp \
		../CeleX/frontpanel/frontpanel.cpp \
		../CeleX/configproc/hhxmlreader.cpp \
		../CeleX/configproc/hhwireincommand.cpp \
		../CeleX/configproc/hhsequencemgr.cpp \
		../CeleX/configproc/hhdelaycommand.cpp \
		../CeleX/configproc/hhcommand.cpp 
OBJECTS       = xbase.o \
		tinyxmlparser.o \
		tinyxmlerror.o \
		tinyxml.o \
		tinystr.o \
		celex5.o \
		celex4.o \
		frontpanel.o \
		hhxmlreader.o \
		hhwireincommand.o \
		hhsequencemgr.o \
		hhdelaycommand.o \
		hhcommand.o

TARGET        = libCeleX.so
TARGETA       = libCeleX.a


first: all
####### Build rules

$(TARGET):  $(OBJECTS) $(SUBLIBS) $(OBJCOMP)  
	-$(DEL_FILE) $(TARGET)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(OBJCOMP)


staticlib: $(TARGETA)

$(TARGETA):  $(OBJECTS) $(OBJCOMP) 
	-$(DEL_FILE) $(TARGETA) 
	$(AR) $(TARGETA) $(OBJECTS)

all: Makefile $(TARGET)

clean: compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


distclean: clean 
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) $(TARGET0) $(TARGET1) $(TARGET2) $(TARGETA)
	-$(DEL_FILE) Makefile


####### Sub-libraries
compiler_clean: 

####### Compile
xbase.o: ../CeleX/base/xbase.cpp ../CeleX/base/xbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o xbase.o ../CeleX/base/xbase.cpp

tinyxmlparser.o: ../CeleX/configproc/tinyxml/tinyxmlparser.cpp ../CeleX/configproc/tinyxml/tinyxml.h \
		../CeleX/configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxmlparser.o ../CeleX/configproc/tinyxml/tinyxmlparser.cpp

tinyxmlerror.o: ../CeleX/configproc/tinyxml/tinyxmlerror.cpp ../CeleX/configproc/tinyxml/tinyxml.h \
		../CeleX/configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxmlerror.o ../CeleX/configproc/tinyxml/tinyxmlerror.cpp

tinyxml.o: ../CeleX/configproc/tinyxml/tinyxml.cpp ../CeleX/configproc/tinyxml/tinyxml.h \
		../CeleX/configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxml.o ../CeleX/configproc/tinyxml/tinyxml.cpp

tinystr.o: ../CeleX/configproc/tinyxml/tinystr.cpp ../CeleX/configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinystr.o ../CeleX/configproc/tinyxml/tinystr.cpp

celex5.o: ../CeleX/eventproc/celex5.cpp ../CeleX/include/celex5/celex5.h \
		../CeleX/include/celextypes.h \
		../CeleX/base/xbase.h \
		../CeleX/driver/CeleDriver.h \
		../CeleX/configproc/hhsequencemgr.h \
		../CeleX/configproc/hhwireincommand.h \
		../CeleX/configproc/hhcommand.h \
		../CeleX/include/celex4/celex4.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o celex5.o ../CeleX/eventproc/celex5.cpp

celex4.o: ../CeleX/eventproc/celex4.cpp ../CeleX/include/celex4/celex4.h \
		../CeleX/include/celextypes.h \
		../CeleX/frontpanel/frontpanel.h \
		../CeleX/frontpanel/okFrontPanelDLL.h \
		../CeleX/base/xbase.h \
		../CeleX/configproc/hhsequencemgr.h \
		../CeleX/include/celex5/celex5.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o celex4.o ../CeleX/eventproc/celex4.cpp

frontpanel.o: ../CeleX/frontpanel/frontpanel.cpp ../CeleX/frontpanel/frontpanel.h \
		../CeleX/frontpanel/okFrontPanelDLL.h \
		../CeleX/base/xbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o frontpanel.o ../CeleX/frontpanel/frontpanel.cpp

hhxmlreader.o: ../CeleX/configproc/hhxmlreader.cpp ../CeleX/configproc/hhxmlreader.h \
		../CeleX/include/celex5/celex5.h \
		../CeleX/include/celextypes.h \
		../CeleX/base/xbase.h \
		../CeleX/configproc/hhwireincommand.h \
		../CeleX/configproc/hhcommand.h \
		../CeleX/configproc/hhdelaycommand.h \
		../CeleX/configproc/hhsequencemgr.h \
		../CeleX/configproc/tinyxml/tinyxml.h \
		../CeleX/configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o hhxmlreader.o ../CeleX/configproc/hhxmlreader.cpp

hhwireincommand.o: ../CeleX/configproc/hhwireincommand.cpp ../CeleX/configproc/hhwireincommand.h \
		../CeleX/configproc/hhcommand.h \
		../CeleX/frontpanel/frontpanel.h \
		../CeleX/frontpanel/okFrontPanelDLL.h \
		../CeleX/base/xbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o hhwireincommand.o ../CeleX/configproc/hhwireincommand.cpp

hhsequencemgr.o: ../CeleX/configproc/hhsequencemgr.cpp ../CeleX/configproc/hhsequencemgr.h \
		../CeleX/include/celex5/celex5.h \
		../CeleX/include/celextypes.h \
		../CeleX/configproc/hhcommand.h \
		../CeleX/configproc/hhxmlreader.h \
		../CeleX/configproc/tinyxml/tinyxml.h \
		../CeleX/configproc/tinyxml/tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o hhsequencemgr.o ../CeleX/configproc/hhsequencemgr.cpp

hhdelaycommand.o: ../CeleX/configproc/hhdelaycommand.cpp ../CeleX/configproc/hhdelaycommand.h \
		../CeleX/configproc/hhcommand.h \
		../CeleX/frontpanel/frontpanel.h \
		../CeleX/frontpanel/okFrontPanelDLL.h \
		../CeleX/base/xbase.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o hhdelaycommand.o ../CeleX/configproc/hhdelaycommand.cpp

hhcommand.o: ../CeleX/configproc/hhcommand.cpp ../CeleX/configproc/hhcommand.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o hhcommand.o ../CeleX/configproc/hhcommand.cpp



