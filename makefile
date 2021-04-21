CC			:= gcc
CC_FLAGS	:= -Ofast -lm

TARGET_DIR	:= build
HEADERS		:= chx.h
OBJS		:= chx.c

.PHONY: all compile build clean

all: build

build:
	@echo "building..."
	mkdir -p ./build
	$(CC) -o $(TARGET_DIR)/chx $(HEADERS) $(OBJS) $(CC_FLAGS)

install:
	sudo cp ./build/chx /usr/bin/

clean:
	@echo "cleaning build directory..."
	mkdir -p ./build
	rm -r ./build/*
