CC=g++
CFLAGS=-Wall -w
LFLAGS=
SRCS=InstrumentHandler.cpp utils.cpp connection.cpp instrumentCreater.cpp instrumentSender.cpp instrumentGateway.cpp /opt/itchTest/Configs/configuration.cpp
OBJS=$(patsubst %.cpp, objs/%.o, $(SRCS))
INCLUDES=
MAIN=InstrumentHandler
LIBS=

.PHONY: depend clean

BUILD_CMD = LOG=$$($(CMD) 2>&1) ; if [ $$? -eq 1 ]; then $(PRINT_ERROR); elif [ "$$LOG" != "" ] ; then $(PRINT_WARNING); else $(PRINT_OK); fi;

all: pre-build $(MAIN) post-build
	@$(call echoGreen, "Finished building main.")

pre-build:
	
post-build:

$(MAIN): $(OBJS)
	@$(call echoYellow, "Linking...")
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

objs/%.o: CMD=$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	
objs/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(BUILD_CMD)

clean:
	@$(call echoYellow, "Cleaning...")
	@$(RM) $(OBJS) *~ $(MAIN)
	@$(call echoGreen, "Finished cleaning.")
	

AWK_CMD=awk '{printf "%-100s %-10s\n", $$1, $$2; }'

NO_COLOR=\033[0m
OK_COLOR=\033[32;01m
ERROR_COLOR=\033[31;01m
WARN_COLOR=\033[33;01m
WHITE_COLOR=\033[37;01m
 
OK_STRING=$(OK_COLOR)[OK]$(NO_COLOR)
ERROR_STRING=$(ERROR_COLOR)[ERRORS]$(NO_COLOR)
WARN_STRING=$(WARN_COLOR)[WARNINGS]$(NO_COLOR)
ACTIVE_STRING=$(OK_COLOR)[ACTIVE]$(NO_COLOR)
INACTIVE_STRING=$(WARN_COLOR)[INACTIVE]$(NO_COLOR)

PRINT_ERROR = printf "$(WHITE_COLOR)$<$(NO_COLOR) $(ERROR_STRING)\n" | $(AWK_CMD) && printf "$(CMD)\n$$LOG\n" && false
PRINT_WARNING = printf "$(WHITE_COLOR)$<$(NO_COLOR) $(WARN_STRING)\n" | $(AWK_CMD) && printf "$(CMD)\n$$LOG\n"
PRINT_OK = printf "$(WHITE_COLOR)$<$(NO_COLOR) $(OK_STRING)\n" | $(AWK_CMD)

PRINT_ACTIVE = printf "$(OK_COLOR)$2$(NO_COLOR) $(ACTIVE_STRING)\n" | $(AWK_CMD)
PRINT_INACTIVE = printf "$(WARN_COLOR)$2$(NO_COLOR) $(INACTIVE_STRING)\n" | $(AWK_CMD)

define echoRed
	tput setaf 1; \
	echo $1; \
	tput sgr0
endef

define echoGreen
	tput setaf 2; \
	echo $1; \
	tput sgr0
endef

define echoYellow
	tput setaf 3; \
	echo $1; \
	tput sgr0
endef

define echoGreenN
	tput setaf 2; \
	echo -n $1; \
	tput sgr0
endef

define echoYellowN
	tput setaf 3; \
	echo -n $1; \
	tput sgr0
endef
