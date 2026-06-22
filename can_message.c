#include <stdint.h>
#include <stdio.h>

/* A received classic CAN frame (2.0A: 11-bit ID, up to 8 data bytes). */
typedef struct {
    uint32_t id;            /* 11-bit identifier            */
    uint8_t  dlc;           /* data length code, 0..8 bytes */
    uint8_t  data[8];
} can_frame_t;

/* Message IDs we expect on the bus. */
#define ID_ENGINE_RPM   0x0C8u
#define ID_WHEEL_SPEED  0x110u

/* Called when the controller signals a frame arrived; decode by ID.
   16-bit values are packed big-endian (high byte first) in data[]. */
void can_on_receive(const can_frame_t *f)
{
    switch (f->id) {
    case ID_ENGINE_RPM: {
        uint16_t rpm = (uint16_t)((f->data[0] << 8) | f->data[1]);
        printf("Engine RPM    = %u\n", rpm);
        break;
    }
    case ID_WHEEL_SPEED: {
        uint16_t kph = (uint16_t)((f->data[0] << 8) | f->data[1]);
        printf("Wheel speed   = %u km/h\n", kph);
        break;
    }
    default:
        printf("Unhandled ID 0x%03X (%u bytes)\n", f->id, f->dlc);
        break;
    }
}

int main(void)
{
    /* Stand-ins for frames a real driver would read from the RX mailbox. */
    can_frame_t a = { .id = ID_ENGINE_RPM,  .dlc = 2, .data = {0x1A, 0xF0} }; /* 6896 */
    can_frame_t b = { .id = ID_WHEEL_SPEED, .dlc = 2, .data = {0x00, 0x55} }; /* 85   */
    can_frame_t c = { .id = 0x200,          .dlc = 1, .data = {0xFF} };       /* unknown */

    can_on_receive(&a);
    can_on_receive(&b);
    can_on_receive(&c);
    return 0;
}
