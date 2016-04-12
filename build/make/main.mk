# Main MAKE file.

# Relative path to build/make folder.
PROJECT_MAKE := $(dir $(lastword $(MAKEFILE_LIST)))

default: setup

.PHONY: default

setup:
	@${PROJECT_MAKE}../bash/setup.sh

.PHONY: setup
