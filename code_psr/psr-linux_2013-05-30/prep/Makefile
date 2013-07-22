# --------------------------------------------------------------------------
# Makefile for OLtrack
#
# U. Acikoz - Fall 2011
#---------------------------------------------------------------------------
CC = g++
ASM = g++
LINK= g++

CINCPATHFLAGS = -IAccessories	\
				-IAcquisition	\
				-IIncludes		\
				-IMain			\
				-IOpenLoop


VPATH		=   Accessories:	\
				Acquisition:	\
				Includes:		\
				Main:			\
				OpenLoop:


LDFLAGS	 = -lpthread
CFLAGS   = -O0 -g3 -Wall -D_FORTIFY_SOURCE=0 -fmessage-length=0 $(CINCPATHFLAGS)
ASMFLAGS = -masm=intel


SRCC = $(wildcard Main/*.cpp Accessories/*.cpp Acquisition/*.cpp OpenLoop/*.cpp)
OBJS = $(SRCC:.cpp=.o)
HEADERS = $(wildcard Accessories/*.h Acquisition/*.h Main/*.h Includes/*.h OpenLoop/*.h)


EXE =	prep

all: $(EXE)
	@echo ---- Build Complete ----

prep:$(OBJS)
	$(LINK) $(LDFLAGS) $(OBJS) -o $@


%.o:%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean: distclean execlean

clean:
	rm -f $(OBJS)

distclean:
	@rm -rvf `find . \( -name "*.o" -o -name "*.dis" -o -name "*.dat" -o -name "*.klm" -o -name "*.m~" -o -name "*.tlm" -o -name "*.log" \) -print`

execlean:
	@rm -rvf $(EXE)


