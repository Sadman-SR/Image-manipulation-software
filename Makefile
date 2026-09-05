# Detect Operating System
ifeq ($(OS),Windows_NT)
    # Windows (MinGW / GCC)
    TARGET = image_editor.exe
    CFLAGS = -Wall -I./iup_tmp -I./iup_tmp/include
    LDFLAGS = -L./iup_tmp -L./iup_tmp/lib -liup -lgdi32 -lcomctl32 -lcomdlg32 -lole32 -luuid
    RM = del /F /Q
   
    # Linux / WSL
    TARGET = image_editor
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