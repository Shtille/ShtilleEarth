# Makefile for deps

TARGET = deps

# Main routine
SUBDIRS_NORMAL = thirdparty scythe
SUBDIRS_CUSTOM = saim

all: $(TARGET)

.PHONY: clean
clean:
	@$(foreach directory, $(SUBDIRS_NORMAL), $(MAKE) -C $(directory) clean ;)
	@$(foreach directory, $(SUBDIRS_CUSTOM), $(MAKE) -C $(directory) clean ;)

.PHONY: help
help:
	@echo available targets: all clean

$(TARGET): $(SUBDIRS_NORMAL) $(SUBDIRS_CUSTOM)

.PHONY: $(SUBDIRS_NORMAL)
$(SUBDIRS_NORMAL):
	@$(MAKE) -C $@ $@

# Declare rules for custom subdirs
.PHONY: saim
saim:
	@$(MAKE) -C $@ $@ USE_THIRDPARTY=NO
