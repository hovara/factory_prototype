CC := gcc
CCFLAGS := -Wall -o3
SRC := *.c
LDLIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
TARGET := factory
all:
	clang-format -i $(SRC) --style=LLVM
	$(CC) $(SRC) $(CCFLAGS) -o $(TARGET) $(LDLIBS)
.PHONY: clean
clean:
	rm $(TARGET)