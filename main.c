#include <stdint.h>
#include <stdio.h>

/* 6 breakpoints => 5 linear intervals.
 * Maps a raw 12-bit ADC count to temperature in tenths of a degree C.
 * Table must be sorted by ascending raw value. */
typedef struct { uint16_t raw; int16_t temp_dC; } point_t;

static const point_t TABLE[6] = {
    {    0, -200 },   /* -20.0 C */
    {  800,    0 },   /*   0.0 C */
    { 1600,  250 },   /*  25.0 C */
    { 2400,  500 },   /*  50.0 C */
    { 3200,  750 },   /*  75.0 C */
    { 4095, 1000 },   /* 100.0 C */
};

/* Piecewise-linear lookup with interpolation and end clamping. */
int16_t adc_to_temp_dC(uint16_t raw)
{
    if (raw <= TABLE[0].raw)   return TABLE[0].temp_dC;          /* clamp low  */
    if (raw >= TABLE[5].raw)   return TABLE[5].temp_dC;          /* clamp high */

    for (uint8_t i = 0; i < 5; ++i) {                            /* 5 intervals */
        const point_t a = TABLE[i];
        const point_t b = TABLE[i + 1];
        if (raw <= b.raw) {
            int32_t dy   = (int32_t)b.temp_dC - a.temp_dC;
            int32_t dx   = (int32_t)b.raw     - a.raw;
            int32_t span = (int32_t)raw       - a.raw;
            return (int16_t)(a.temp_dC + (span * dy + dx / 2) / dx);
        }
    }
    return TABLE[5].temp_dC; /* unreachable */
}

int main(void)
{
    uint16_t tests[] = {0, 400, 1200, 2000, 3600, 5000};
    for (unsigned i = 0; i < 6; ++i) {
        int16_t t = adc_to_temp_dC(tests[i]);
        printf("raw: %4u, temp: %d.%d C\n", tests[i], t / 10, (t < 0 ? -t : t) % 10);
    }
    return 0;
}
