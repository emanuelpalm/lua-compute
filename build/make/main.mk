# Main MAKE file.

# Relative path to build/make folder.
PROJECT_MAKE := $(dir $(lastword $(MAKEFILE_LIST)))

RM = rm -f

default: default-setup default-native
all: all-setup all-native
clean: clean-setup clean-native

.PHONY: default all clean

-include ${PROJECT_MAKE}../env.conf
include ${PROJECT_MAKE}/setup.mk
include ${PROJECT_MAKE}/native.mk
