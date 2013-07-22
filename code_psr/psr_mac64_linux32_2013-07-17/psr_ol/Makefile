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
				-IOLChannel		\
				-IChannel		\
				-IOLChanBck		\
				-Ipsrbitgen     \
				-IOpenLoop

 				
VPATH		=   Accessories:	\
				Acquisition:	\
				Includes:		\
				Main:			\
				OLChannel:		\
				OlChanBck:		\
				Channel:		\
				psrbitgen:      \
				OpenLoop:       
				
	
LDFLAGS	 = -lpthread -pg
CFLAGS   = -O0 -g3 -Wall -pg -D_FORTIFY_SOURCE=0 -fmessage-length=0 $(CINCPATHFLAGS)
ASMFLAGS = -masm=intel


SRCC = $(wildcard Main/*.cpp Accessories/*.cpp Acquisition/*.cpp OLChannel/*.cpp OLChanBck/*.cpp OpenLoop/*.cpp Channel/*.cpp psrbitgen/*.cpp) 
OBJS = $(SRCC:.cpp=.o)
HEADERS = $(wildcard Accessories/*.h Acquisition/*.h Main/*.h OLChannel/*.h OLChanBck/*.h Includes/*.h OpenLoop/*.h Channel/*.h psrbitgen/*.h)			
			

EXE =	OLtrack

all: $(EXE)
	@echo ---- Build Complete ----

OLtrack:$(OBJS)
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
	

