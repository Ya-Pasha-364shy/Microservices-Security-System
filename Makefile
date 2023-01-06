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
BINARIES_SERVICES=$(WORKDIR)/bin/services
BINARIES_HELPERS=$(WORKDIR)/bin/helpers

# COMPILER
CC=gcc

# FLAGS
FLAGS=-c -s -Wall -Wextra -O2

all: all_services

# build_helpers_message:
# 	$(CC) $(FLAGS) $(WORKDIR_HELPERS)/helpers-message.c \
# 	-o $(BINARIES_HELPERS)/helpers-message.o

build_helpers_common:
	$(CC) $(FLAGS) $(WORKDIR_HELPERS)/helpers-common.c \
	-o $(BINARIES_HELPERS)/helpers-common.o

build_backup_service_fs_iter: build_helpers_common
	$(CC) $(FLAGS) $(WORKDIR_SERVICES)/backup-service/backup-service-fs-iteration.c \
	-o $(BINARIES_SERVICES)/backup-service-fs-iteration.o

build_backup_service: build_backup_service_fs_iter
	$(CC) $(FLAGS) $(WORKDIR_SERVICES)/backup-service/backup-service-main.c \
	-o $(BINARIES_SERVICES)/backup-service-main.o

build_service_manager: build_backup_service
	$(CC) $(FLAGS) $(WORKDIR_SERVICES)/service-manager-main.c \
	-o $(BINARIES_SERVICES)/service-manager.o

all_services: build_service_manager
	$(CC) $(BINARIES_SERVICES)/service-manager.o \
	$(BINARIES_HELPERS)/helpers-common.o \
	-o $(BINARIES_SERVICES)/../service-manager

	$(CC) $(BINARIES_SERVICES)/backup-service-main.o \
	$(BINARIES_SERVICES)/backup-service-fs-iteration.o \
	-o $(BINARIES_SERVICES)/../backup-service

clean:
	rm -f $(BINARIES_SERVICES)/../service-manager $(BINARIES_SERVICES)/../backup-service
