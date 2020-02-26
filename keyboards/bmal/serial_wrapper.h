#include <inttypes.h>
#include <stddef.h>

void Serial1_begin(unsigned long);
void Serial1_end(void);
int Serial1_available(void);
int Serial1_peek(void);
int Serial1_read(void);
void Serial1_flush(void);
size_t Serial1_write(uint8_t);
void Serial1_println(char *str);