# Makefile for project setup.

default-setup: setup
all-setup: setup

setup:
	@${PROJECT_MAKE}../bash/setup.sh

clean-setup:
	$(foreach F,$(wildcard ${PROJECT_MAKE}../*.conf),$(RM) $F;)

.PHONY: default-setup all-setup setup clean-setup


