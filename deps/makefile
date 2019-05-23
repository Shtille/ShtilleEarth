# Makefile for deps

TARGET = deps

# Main routine
SUBDIRS_CUSTOM = scythe-thirdparty
SUBDIRS_NORMAL = scythe saim

all: $(TARGET)

.PHONY: clean
clean:
	@$(foreach directory, $(SUBDIRS_CUSTOM), $(MAKE) -C $(directory) clean ;)
	@$(foreach directory, $(SUBDIRS_NORMAL), $(MAKE) -C $(directory) clean ;)

.PHONY: help
help:
	@echo available targets: all clean

$(TARGET): $(SUBDIRS_CUSTOM) $(SUBDIRS_NORMAL)

.PHONY: $(SUBDIRS_NORMAL)
$(SUBDIRS_NORMAL):
	@$(MAKE) -C $@ $@

# Declare rules for custom subdirs
.PHONY: scythe-thirdparty
scythe-thirdparty:
	@$(MAKE) -C scythe-thirdparty thirdparty
