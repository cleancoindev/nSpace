include		../../Makefile.rules

# Source files
OBJS	:= nspcn.o
LIBS	:= -lnspcl -ladtl -lccl -lsysl
LDFLAGS	:= $(LDFLAGS) -L$(OUTLOC) -Wl,--version-script=../../lib/ccl/ccl.version

# Library
all							:	$(OUTLOC)libnspcn.so

$(OUTLOC)libnspcn.so		:	$(OBJS)
									$(LD) $(LDFLAGS) -o $(OUTLOC)libnspcn.so $(OBJS) $(LIBS)

clean							:
									rm -f *.o
									rm -f $(OUTLOC)libnspcn.so

