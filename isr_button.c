#include <stdint.h>
#include <stdio.h>

#define DEBOUNCE_MS 20u

extern volatile uint32_t millis;       /* incremented by a 1 kHz timer ISR */
volatile uint8_t button_event = 0;     /* set by ISR, cleared by main loop */

/* GPIO falling-edge interrupt (button pulls the pin to GND when pressed). */
void Button_IRQHandler(void)
{
    static uint32_t last = 0;
    uint32_t now = millis;

    clear_button_irq_flag();           /* acknowledge the hardware IRQ */

    if (now - last >= DEBOUNCE_MS) {   /* ignore contact bounce in the window */
        last = now;
        button_event = 1;              /* hand the event to the main loop */
    }
}

int main(void)
{
    for (;;) {
        if (button_event) {
            button_event = 0;
            /* do the real work here, not in the ISR */
            printf("Button pressed (t=%u ms)\n", millis);
        }
    }
}
