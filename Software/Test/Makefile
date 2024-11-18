# define colors
RED=\033[0;31m
GREEN=\033[0;32m
ORANGE=\033[0;33m
BLUE=\033[0;34m
PURPLE=\033[0;35m
CYAN=\033[0;36m
LIGHTGRAY=\033[0;37m
DARKGRAY=\033[1;30m
YELLOW=\033[0;33m
NC=\033[0m # No Color

# default project configuration
TEST ?= main
PROJECT_TARGET ?= stm32f446ret

# source and include directories
PROJECT_C_SOURCES = Tests/Src
PROJECT_C_INCLUDES = Tests/Inc

# build and driver directories
PROJECT_BUILD_DIR = ../Embedded-Sharepoint/build
BUILD_MAKEFILE_DIR = ../Embedded-Sharepoint

# ensure all paths are absolute
MAKEFILE_DIR = $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES)/$(TEST).c)
PROJECT_C_INCLUDES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_INCLUDES))
PROJECT_BUILD_DIR := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_BUILD_DIR))

# export variables for sub-makes
export PROJECT_TARGET
export PROJECT_C_SOURCES
export PROJECT_C_INCLUDES
export PROJECT_BUILD_DIR
export TEST

# check if bear is installed
BEAR_INSTALLED := $(shell command -v bear >/dev/null 2>&1 && echo yes || echo no)

ifeq ($(MAKECMDGOALS),)
default: conditional_bear_make
else ifeq ($(MAKECMDGOALS),all)
all: conditional_bear_make
else
%:
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) $(MAKECMDGOALS)
endif

.PHONY: conditional_bear_make
conditional_bear_make:
	@echo "Making STM32 build for ${BLUE}TEST =${PURPLE} ${TEST}${NC}"
ifeq ($(BEAR_INSTALLED),yes)
	@echo "Bear is installed, running with bear..."
	bear $(MAKE) -C $(BUILD_MAKEFILE_DIR) all ; \
	mv compile_commands.json ../.vscode/compile_commands.json
else
	@echo "Bear is not installed, running normal make..."
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) all
	@echo "${BLUE}Compiled for DataAcq Environment! Jolly Good :D${NC}"
endif

# generate documentation
.PHONY: docs
docs:
	cd .. && mkdocs serve

# help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  docs     - Generate documentation using mkdocs."
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) help
