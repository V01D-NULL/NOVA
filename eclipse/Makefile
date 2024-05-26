TARGET = eclipse.elf

CFLAGS = -ffreestanding -Wall -Wextra -c
LDFLAGS = -nostdlib -z max-page-size=0x1000

# Use find to get all .c and .S files
C_FILES := $(shell find . -name '*.c')
S_FILES := $(shell find . -name '*.S')
OBJS := $(C_FILES:.c=.o) $(S_FILES:.S=.o)

$(TARGET): $(OBJS) link.ld
	ld $(LDFLAGS) -T link.ld $(OBJS) -o $(TARGET)

%.o: %.c
	gcc $(CFLAGS) $< -o $@

%.o: %.S
	gcc $(CFLAGS) -x assembler-with-cpp $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
