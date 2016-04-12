# Main MAKE file.

# Relative path to build/make folder.
PROJECT_MAKE := $(dir $(lastword $(MAKEFILE_LIST)))

RM = rm -f

-include ${PROJECT_MAKE}../env.conf
include ${PROJECT_MAKE}/setup.mk

default: default-setup
all: all-setup
clean: clean-setup

.PHONY: default all clean
