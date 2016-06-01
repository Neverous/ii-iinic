#include <avr/interrupt.h>
#include <stdint.h>

void __inc16(volatile uint16_t *value)
{
    asm(
        "lds r24, %0\n\t"
        "inc r24\n\t"
        "sts %0, r24\n\t"
        "brne 1f\n\t"

        "lds r24, %0+1\n\t"
        "inc r24\n\t"
        "sts %0+1, r24\n\t"

    "1:\n\t"
    : "=m" (*value)
    : "m" (*value)
    : "r24"
    );
}

void __inc24(volatile __uint24 *value)
{
    asm(
        "lds r24, %0\n\t"
        "inc r24\n\t"
        "sts %0, r24\n\t"
        "brne 1f\n\t"

        "lds r24, %0+1\n\t"
        "inc r24\n\t"
        "sts %0+1, r24\n\t"
        "brne 1f\n\t"

        "lds r24, %0+2\n\t"
        "inc r24\n\t"
        "sts %0+2, r24\n\t"

    "1:\n\t"
    : "=m" (*value)
    : "m" (*value)
    : "r24"
    );
}

void __inc32(volatile uint32_t *value)
{
    uint8_t reg;
    asm volatile(
        "lds %0, %1\n\t"
        "inc %0\n\t"
        "sts %1, %0\n\t"
        "brne 1f\n\t"

        "lds %0, %1+1\n\t"
        "inc %0\n\t"
        "sts %1+1, %0\n\t"
        "brne 1f\n\t"

        "lds %0, %1+2\n\t"
        "inc %0\n\t"
        "sts %1+2, %0\n\t"
        "brne 1f\n\t"

        "lds %0, %1+3\n\t"
        "inc %0\n\t"
        "sts %1+3, %0\n\t"

    "1:\n\t"
    : "=&a" (reg)
    : "m" (*value)
    );
}


static volatile uint32_t timing_high;
ISR(TIMER1_OVF_vect)
{
    __inc32(&timing_high);
    //++ timing_high;
}

    uint32_t siema;
void main(void)
{
    __inc32(&siema);
}
