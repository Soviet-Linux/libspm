

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



CFLAGS = -Wall -g -fPIC -O2 -Wextra -L./bin -Iinclude

LIBS = lib/* -lcurl -lsqlite3 -lm 

# change these to proper directories where each file should be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
INCDIR   = include

DEVDIR = dev


SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

FMT_DIR = formats

MEMCHECK = 0


all: $(BINDIR)/$(LIBOUT)
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
	$(CC) $(CFLAGS) -DSTATIC ${FMT_DIR}/*/* ${DEVDIR}/test.c $(LIBS) -o bin/spm-test -lspm -L./bin


check-data:
	@echo "Checking data..."
	bin/spm-test data
	@echo "Data test passed"

check-ecmp:
	bin/spm-test ecmp
	@echo "ECMP test passed"

check: test check-data check-ecmp
	@echo "All Tests Passed"



cccp:
	$(CC) $(CFLAGS) ${DEVDIR}/cccp.c $(LIBS) -o bin/cccp -lspm -L./bin

direct:
	$(CC) $(CFLAGS) $(SRCS) $(LIBS) -g -shared -fPIC -o $(LIBOUT)

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
	@if [ ! -d /usr/include/spm ]; then mkdir /usr/include/spm; fi
	for i in include/*; do install -vDm 755 $$i /usr/include/spm/; done
	install -vDm 755 $(BINDIR)/$(LIBOUT) $(DESTDIR)/lib/$(LIBOUT) 
	install  $(BINDIR)/plugins/ecmp.so -vDm 755 $(DESTDIR)/var/cccp/plugins/ecmp.so


	


