TARGETS	= test_varbuf

.PHONY: clean _dirs_

ifndef OS
OS= $(shell uname -s)
endif
ifndef ARCH
ARCH= $(shell arch)
endif
 
CPPFLAGS += -I.
CPPFLAGS += -I/usr/local/include

CFLAGS += -Werror -Wall
CFLAGS += -g

LDFLAGS	+= -L/usr/local/lib

all: _dirs_ $(TARGETS)

_dirs_:
ifdef	DIRS
	for d in $(DIRS); do if test -d $$d ; then (cd $$d ; $(DEFS) make); fi ; done
endif

$(TARGETS): $(OBJS)

clean:
	-rm -rf a.out *.dSYM *.o
ifneq ($(RMFILES),)
	-rm -rf $(RMFILES)
endif
ifdef	DIRS
	for d in $(DIRS) ; do (cd $$d ; make clean); done
endif
ifneq ($(TARGETS),)
	-rm -f $(TARGETS)
endif
ifneq ($(TEST_TARGETS),)
	-rm -f $(TEST_TARGETS)
endif

distclean: clean
	-rm -f config.cache config.status config.log .depend
	if test -f Makefile.ini ; then rm -f Makefile ; fi

show-options:
	@echo CPPFLAGS =$(CPPFLAGS)
	@echo   CFLAGS =$(CFLAGS)
	@echo  LDFLAGS =$(LDFLAGS)
	@echo   LDLIBS =$(LDLIBS)

test_varbuf: test_varbuf.o varbuf.o

