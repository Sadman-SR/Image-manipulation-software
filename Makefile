# Detect Operating System
ifeq ($(OS),Windows_NT)
    TARGET = image_editor.exe
    CC = gcc
    CFLAGS = -Wall -I./iup_tmp -I./iup_tmp/include
    LDFLAGS = -L./iup_tmp -L./iup_tmp/lib -liup -lgdi32 -lcomctl32 -lcomdlg32 -lole32 -luuid
    RM = del /F /Q
else
    TARGET = image_editor
    CC = gcc
    CFLAGS = -Wall -I./iup_tmp -I./iup_tmp/include $(shell pkg-config --cflags gtk+-3.0)
    LDFLAGS = -L./iup_tmp -L./iup_tmp/lib -Wl,-rpath,'$$ORIGIN/iup_tmp' -Wl,-rpath,'$$ORIGIN/iup_tmp/lib' -liup $(shell pkg-config --libs gtk+-3.0) -lm
    RM = rm -f
endif

SRCS = main.c gui.c image_manage.c image_process.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: all clean