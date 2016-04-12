# Main MAKE file.

# Relative path to build/make folder.
PROJECT_MAKE := $(dir $(lastword $(MAKEFILE_LIST)))

-include ${PROJECT_MAKE}../env.conf

default: setup

.PHONY: default

setup:
	@${PROJECT_MAKE}../bash/setup.sh

.PHONY: setup

clean-setup:
	$(foreach F,$(wildcard ${PROJECT_MAKE}../*.conf),$(RM) $F;)
