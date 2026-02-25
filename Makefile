APP := how
CC := cc
CSTD := c99

SRC_DIR := src
INC_DIR := include
LIB_DIR := libs
BUILD := build

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
INSTALL ?= install
INSTALL_BIN ?= $(INSTALL) -m 0755

CFLAGS := -std=$(CSTD) -O2 -Wall -Wextra -Wpedantic -I$(INC_DIR) -I$(LIB_DIR)/yyjson
LDFLAGS := -lssl -lcrypto

SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
SRCS += $(LIB_DIR)/yyjson/yyjson.c

OBJS := $(patsubst %.c,$(BUILD)/%.o,$(SRCS))

.PHONY: all clean run format install uninstall
all: $(APP)

$(APP): $(OBJS)
	@printf "LD %s\n" $@
	@$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@printf "CC %s\n" $<
	@$(CC) $(CFLAGS) -c $< -o $@

run: $(APP)
	@./$(APP)

install: $(APP)
	@printf "IN %s\n" "$(DESTDIR)$(BINDIR)/$(APP)"
	@$(INSTALL) -d "$(DESTDIR)$(BINDIR)"
	@$(INSTALL_BIN) "$(APP)" "$(DESTDIR)$(BINDIR)/$(APP)"

uninstall:
	@printf "UN %s\n" "$(DESTDIR)$(BINDIR)/$(APP)"
	@rm -f "$(DESTDIR)$(BINDIR)/$(APP)"

clean:
	@rm -rf $(BUILD) $(APP)
