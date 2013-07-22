# --------------------------------------------------------------------------
# Makefile for code_psr
#
# Junlan 2013_07_22
#---------------------------------------------------------------------------
CC = g++
ASM = g++
LINK= g++

# Uncomment options below for your particular OS:

#64 bit mac defaults, comment this part if you are using 32 bit linux:
CINCPATHFLAGS =                 -IAccessories           \
                                -IAccessories_mac	\
              		        -IAcquisition	        \
				-IIncludes		\
				-IMain			\
				-IOpenLoop           

VPATH		=               Accessories:            \
                                Accessories_mac:	\
				Acquisition:	        \
				Includes:		\
				Main:			\
				OpenLoop:
LDFLAGS	 = -lpthread -m32
CFLAGS   = -m32 -g3 -Wall -D_FORTIFY_SOURCE=0 -fmessage-length=0 $(CINCPATHFLAGS)
ASMFLAGS = -masm=intel
SRCC = $(wildcard Main/*.cpp Accessories/*.cpp Accessories_mac/*.cpp Acquisition/*.cpp OpenLoop/*.cpp)
OBJS = $(SRCC:.cpp=.o)
HEADERS = $(wildcard Accessories/*.h Acquisition/*.h Main/*.h Includes/*.h OpenLoop/*.h)


#Uncomment for 32 bit linux
#CINCPATHFLAGS =                 -IAccessories          \
#                                -IAccessories_linux	\
#              		         -IAcquisition	        \
#				 -IIncludes		\
#				 -IMain			\
#				 -IOpenLoop           
#VPATH		=               Accessories:            \
#                               Accessories_linux:	\
#				Acquisition:	        \
#				Includes:		\
#				Main:			\
#				OpenLoop:
#LDFLAGS	 = -lpthread
#CFLAGS   = -O0 -g3 -Wall -D_FORTIFY_SOURCE=0 -fmessage-length=0 $(CINCPATHFLAGS)
#ASMFLAGS = -masm=intel
#SRCC = $(wildcard Main/*.cpp Accessories/*.cpp Accessories_linux/*.cpp Acquisition/*.cpp OpenLoop/*.cpp)
#OBJS = $(SRCC:.cpp=.o)
#HEADERS = $(wildcard Accessories/*.h Acquisition/*.h Main/*.h Includes/*.h OpenLoop/*.h)

 
#output:
EXE =	prep.out

all: $(EXE)
	@echo ---- Build Complete ----

prep.out:$(OBJS)
	$(LINK) $(LDFLAGS) $(OBJS) -o $@


%.o:%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean: distclean

clean:
	rm -f $(OBJS) prep 
distclean:
	@rm -rvf `find . \( -name "*.o" -o -name "*.dis" -o -name "*.dat" -o -name "*.klm" -o -name "*.m~" -o -name "*.tlm" -o -name "*.log" \) -print`

#execlean:
#	@rm -rvf $(EXE)


