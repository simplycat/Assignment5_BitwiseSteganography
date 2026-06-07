CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = steg
SRCS = main.c ppm.c steg.c utils.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) encoded.ppm decoded.bin

# Functional testing
test: $(TARGET)
	@echo "=== Building ==="
	@make clean
	@make
	@echo "\n=== Testing encode/decode roundtrip ==="
	@printf 'Hello Stego!' > payload.bin
	@./$(TARGET) encode test_image.ppm payload.bin encoded.ppm
	@./$(TARGET) decode encoded.ppm decoded.bin
	@diff payload.bin decoded.bin && echo "SUCCESS: payload.bin == decoded.bin" || echo "FAIL"
	@echo "\n=== Testing error cases ==="
	@./$(TARGET) encode test_image.ppm payload.bin test_image.ppm 2>&1 || true
	@rm -f payload.bin encoded.ppm decoded.bin
	@echo "\n=== All tests completed ==="
