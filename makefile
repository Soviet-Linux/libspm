

# ------------------------------------------------
# Generic Makefile
#
# Author: yanick.rochon@gmail.com
# Date  : 2011-08-10
#
# Changelog :
#   2010-11-05 - first version
#   2011-08-10 - added structure : sources, objects, binaries
#				thanks to http://stackoverflow.com/users/128940/beta
#   2017-04-24 - changed order of linker params
# ------------------------------------------------

# -------------------
# CCCP Makefile
# Modified by: PKD
#--------------------

LIBOUT = libspm.so


CC = gcc
CPP = g++

ODIR = obj
SDIR = src



CFLAGS = -Wall -fPIC -O2 -Wextra -L./bin -Iinclude 
DBGFLAGS = -g -fsanitize=address

# set local lib to lib/*/*.a
LOCAL_LIBS = $(wildcard lib/*/*.a)
LIBS = ${LOCAL_LIBS} -lgit2 -lsqlite3 -lcurl -lm -lcrypto

# change these to proper directories where each file should be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
INCDIR   = include

DEVDIR = test


SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

FMT_DIR = formats

MEMCHECK = 0


all: libs $(BINDIR)/$(LIBOUT) formats
	@echo "BUILD SUCESSFUL"

$(BINDIR)/$(LIBOUT): $(OBJECTS)
	@$(CC) $(OBJECTS) $(LIBS) $(LFLAGS) -o $@ -shared
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c

	@if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi
	@if [ ! -d $(BINDIR) ]; then mkdir $(BINDIR); fi

	$(CC) $(CFLAGS) -c $< -o $@ -D MEMCHECK=$(MEMCHECK)
	@echo "Compiled "$<" successfully!"



test:
	$(CC) $(CFLAGS) -DSTATIC ${FMT_DIR}/*/* ${DEVDIR}/spm.c ${DEVDIR}/test.c $(LIBS) -o bin/spm-test -lspm -L./bin -D MEMCHECK=$(MEMCHECK)
	$(CC) $(CFLAGS) -DSTATIC ${FMT_DIR}/*/* ${DEVDIR}/package.c ${DEVDIR}/test.c $(LIBS) -o bin/package-test -lspm -L./bin -D MEMCHECK=$(MEMCHECK)
	@echo "Test binary created"

check-all:
	bin/spm-test all
	@if [ $$? -gt 0 ]; then echo "Error Tests Failed"; else echo "All good"; fi

check: test check-all
	@echo "All Tests Passed"


# This will conflict with other artifacts so you should run make clean before `make debug` and after you're done
debug: CFLAGS += $(DBGFLAGS)
debug: MEMCHECK = 1
debug: libs $(BINDIR)/$(LIBOUT) formats
	@echo "Build done (debug)"

libs:
	for i in $(LOCAL_LIBS); do make -C $$(dirname $$i) all; done

direct:
	$(CC) $(CFLAGS) $(SRCS) $(LIBS) -g -shared -fPIC -o $(BINDIR)/$(LIBOUT)

formats:
	@echo "Building formats..."
	@echo $(FMT_DIR)/*
	[ -d $(BINDIR)/plugins ] || mkdir $(BINDIR)/plugins
	for i in $(FMT_DIR)/*; do \
		echo "Building $$i"; \
		if [ -d $$i ]; then \
			$(CC) $(CFLAGS) -shared -fPIC $$i/*.c -o $(BINDIR)/plugins/$$(basename $$i).so; \
		fi; \
	done

.PHONY: clean test formats

clean:
	rm -f $(ODIR)/*.o $(BINDIR)/$(LIBOUT) $(BINDIR)/plugins/*.so 

install: $(BINDIR)/$(LIBOUT)
	@if [ ! -d $(DESTDIR)/usr/include/spm ]; then mkdir -p $(DESTDIR)/usr/include/spm; fi
	for i in include/*; do install -vDm 755 $$i $(DESTDIR)/usr/include/spm/; done
	install -vDm 755 $(BINDIR)/$(LIBOUT) $(DESTDIR)/usr/lib/$(LIBOUT)
	install  $(BINDIR)/plugins/ecmp.so -vDm 755 $(DESTDIR)/var/cccp/plugins/ecmp.so