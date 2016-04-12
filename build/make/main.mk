# Main MAKE file.

# Used command line utilities.
RM=rm -f

# Relative path to build/make folder.
PROJECT_MAKE := $(dir $(lastword $(MAKEFILE_LIST)))

default: setup

.PHONY: default

setup:
	@${PROJECT_MAKE}../bash/setup.sh

.PHONY: setup

clean-setup:
	$(foreach F,$(wildcard ${PROJECT_MAKE}../*.conf),$(RM) $F;)
