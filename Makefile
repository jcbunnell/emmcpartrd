# the compiler: gcc for C program, define as g++ for C++
  CC = gcc

  # compiler flags:
  #  -g    adds debugging information to the executable file
  #  -Wall turns on most, but not all, compiler warnings
  CFLAGS  = -g -Wall

  # the build target executable:
  TARGET = listemmc

  all: binaries/debug/$(TARGET)

  binaries/debug/$(TARGET): source/src/$(TARGET).c
  	$(CC) $(CFLAGS) -o binaries/debug/$(TARGET) source/src/$(TARGET).c

  clean:
  	$(RM) binaries/debug/$(TARGET)
