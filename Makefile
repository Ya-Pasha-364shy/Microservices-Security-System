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

### COMMON HELPERS

build_helpers_parser:
	$(CC) $(FLAGS) $(WORKDIR_HELPERS)/helpers_parser.c \
	-o $(BINARIES_HELPERS)/helpers_parser.o

build_helpers_common: build_helpers_parser
	$(CC) $(FLAGS) $(WORKDIR_HELPERS)/helpers_common.c \
	-o $(BINARIES_HELPERS)/helpers_common.o

################

### FIREWALL SERVICE BUILD

FIREWALL_FLAGS=-c -g -O2 -Wextra
BINARIES_FIREWALL_HELPERS=$(BINARIES_SERVICES)/firewall_service
WORKDIR_FIREWALL_SERVICES=$(WORKDIR_SERVICES)/firewall_service

build_firewall_service_helpers:
	$(CC) $(FIREWALL_FLAGS) $(WORKDIR_SERVICES)/firewall_service/helpers_firewall_common.c \
	-o $(BINARIES_HELPERS)/helpers_firewall_common.o

build_firewall_service: build_firewall_service_helpers
	$(CC) $(FIREWALL_FLAGS) -D_GNU_SOURCE $(WORKDIR_SERVICES)/firewall_service/firewall_service_main.c \
	-o $(BINARIES_SERVICES)/firewall_service/firewall_service_main.o

	$(CC) $(BINARIES_SERVICES)/firewall_service/firewall_service_main.o $(BINARIES_HELPERS)/helpers_parser.o \
	$(BINARIES_HELPERS)/helpers_firewall_common.o -o $(BINARIES_SERVICES)/../firewall_service -lpcap

#####################

### BACKUP SERVICE BUILD

BACKUP_FLAGS=-c -g -O2 -Wextra
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
	$(CC) $(BACKUP_FLAGS) $(WORKDIR_BACKUP_SERVICES)/helpers_backup_common.c -o \
	$(BINARIES_BACKUP_HELPERS)/helpers_backup_common.o

build_main_loop: build_helpers
	$(CC) $(BACKUP_FLAGS) -D_GNU_SOURCE $(WORKDIR_BACKUP_SERVICES)/backup_service_main.c -o \
	$(BINARIES_BACKUP_HELPERS)/backup_service_main.o -lpthread 

build_backup_service: build_main_loop
	$(CC) $(BINARIES_BACKUP_HELPERS)/backup_service_main.o $(BINARIES_HELPERS)/helpers_common.o $(BINARIES_HELPERS)/helpers_event_queue.o $(BINARIES_HELPERS)/helpers_hash_table.o $(BINARIES_HELPERS)/helpers_logger.o $(BINARIES_BACKUP_HELPERS)/helpers_backup_common.o $(BINARIES_HELPERS)/helpers_parser.o \
	-o $(BINARIES_SERVICES)/../backup_service

########################

build_service_manager: build_backup_service build_firewall_service
	$(CC) $(FLAGS) $(WORKDIR_SERVICES)/service_manager_main.c \
	-o $(BINARIES_SERVICES)/service_manager.o
	
	rm -f $(BINARIES_BACKUP_HELPERS)/*.o
	rm -f $(BINARIES_FIREWALL_HELPERS)/*.o

all_services: build_service_manager
	$(CC) $(BINARIES_SERVICES)/service_manager.o \
	$(BINARIES_HELPERS)/helpers_common.o \
	-o $(BINARIES_SERVICES)/../service_manager

	rm -rf $(BINARIES_SERVICES)/*.o $(BINARIES_HELPERS)/*.o

#############################################

check:
	ls -la $(BINARIES_SERVICES)/../ > ./test.txt
	bash ./.github/check_proccess.sh

clean:
	rm -f $(BINARIES_SERVICES)/../service_manager $(BINARIES_SERVICES)/../backup_service \
	      $(BINARIES_SERVICES)/../firewall_service
