WORKDIR=$(shell pwd)

# HEADERS
WORKDIR_H=$(WORKDIR)/include
WORKDIR_HELPERS_H=$(WORKDIR)/include/helpers
WORKDIR_SERVICES_H=$(WORKDIR)/include/services

# SOURCES
WORKDIR_SERVICES=$(WORKDIR)/services

# HELPERS
WORKDIR_HELPERS=$(WORKDIR_SERVICES)/helpers

# BINARIES
BINARIES=$(WORKDIR)/bin

# HELPERS BINARIES
BINARIES_HELPERS=$(WORKDIR)/bin/helpers

# COMPILER
CC=gcc

# FLAGS
FLAGS=-s -Wall -Wextra -O2 

all: all_services

all_services:
	$(CC) $(FLAGS) $(WORKDIR_SERVICES)/service-manager-main.c \
	$(WORKDIR_SERVICES_H)/service-manager-main.h \
	-o $(BINARIES)/service-manager
	
	$(CC) $(FLAGS) $(WORKDIR_SERVICES)/backup-service/backup-service-main.c \
	$(WORKDIR_SERVICES)/backup-service/backup-service-fs-iteration.c \
	$(WORKDIR_HELPERS)/helpers-common.c \
	$(WORKDIR_SERVICES_H)/backup-service/backup-service-main.h \
	-o $(BINARIES)/backup-service

helpers_build:
	$(CC) $(WORKDIR_HELPERS)/helpers-message.h -o $(BINARIES_HELPERS)/helpers-message.o

clean:
	rm -f $(BINARIES)/service-manager $(BINARIES)/backup-service
