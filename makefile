CC			:= gcc
CC_FLAGS	:= -O0 -lm

TARGET_DIR	:= build
HEADERS		:= chx.h
OBJS		:= chx.c

.PHONY: all compile build clean

all: build

build:
	@echo "building..."
	$(CC) -o $(TARGET_DIR)/chx $(HEADERS) $(OBJS) $(CC_FLAGS)

clean:
	@echo "cleaning build directory..."
	rm -r ./build/*
