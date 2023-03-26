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

###### BUILD ALL SERVICES and HELPERS HERE !

all: all_services

build_helpers_common:
	$(CC) $(FLAGS) $(WORKDIR_HELPERS)/helpers_common.c \
	-o $(BINARIES_HELPERS)/helpers_common.o

### BACKUP SERVICE BUILD

BACKUP_FLAGS=-c -g -O2
BINARIES_BACKUP_HELPERS=$(BINARIES_SERVICES)/backup_service
WORKDIR_BACKUP_SERVICES=$(WORKDIR_SERVICES)/backup_service

build_logger_api: build_helpers_common
	$(CC) $(BACKUP_FLAGS) $(WORKDIR_HELPERS)/helpers_logger.c -o \
	$(BINARIES_HELPERS)/helpers_logger.o

build_hash_table_api: build_logger_api
	$(CC) $(BACKUP_FLAGS) $(WORKDIR_HELPERS)/helpers_hash_table.c -o \
	$(BINARIES_HELPERS)/helpers_hash_table.o

build_event_queue: build_hash_table_api
	$(CC) $(BACKUP_FLAGS) $(WORKDIR_HELPERS)/helpers_event_queue.c -o \
	$(BINARIES_HELPERS)/helpers_event_queue.o

build_helpers: build_event_queue
	$(CC) $(BACKUP_FLAGS) -lpthread $(WORKDIR_BACKUP_SERVICES)/helpers_backup_common.c -o \
    $(BINARIES_BACKUP_HELPERS)/helpers_backup_common.o 

build_main_loop: build_helpers
	$(CC) $(BACKUP_FLAGS) -D_GNU_SOURCE -lpthread $(WORKDIR_BACKUP_SERVICES)/backup_service_main.c -o \
	$(BINARIES_BACKUP_HELPERS)/backup_service_main.o

build_backup_service: build_main_loop
	$(CC) $(BINARIES_BACKUP_HELPERS)/backup_service_main.o $(BINARIES_HELPERS)/helpers_common.o $(BINARIES_HELPERS)/helpers_event_queue.o $(BINARIES_HELPERS)/helpers_hash_table.o $(BINARIES_HELPERS)/helpers_logger.o $(BINARIES_BACKUP_HELPERS)/helpers_backup_common.o -o $(BINARIES_SERVICES)/../backup_service
	rm -f $(BINARIES_BACKUP_HELPERS)/*.o

########################

build_service_manager: build_backup_service 
	$(CC) $(FLAGS) $(WORKDIR_SERVICES)/service_manager_main.c \
	-o $(BINARIES_SERVICES)/service_manager.o

all_services: build_service_manager
	$(CC) $(BINARIES_SERVICES)/service_manager.o \
	$(BINARIES_HELPERS)/helpers_common.o \
	-o $(BINARIES_SERVICES)/../service_manager

	rm -rf $(BINARIES_SERVICES)/*.o $(BINARIES_HELPERS)/*.o

#############################################

check:
	ls -la $(BINARIES_SERVICES)/../ > test.txt
	bash check_proccess.sh

clean:
	rm -f $(BINARIES_SERVICES)/../service_manager $(BINARIES_SERVICES)/../backup_service
