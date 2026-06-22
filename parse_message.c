/*
 * parse_message.c  -  compact protocol parser
 *   Frame: AA | COUNT | CMD | LEN | DATA[LEN] | CRC8 | 55
 *   CRC-8/MAXIM-DOW over COUNT..last DATA byte; valid when crc^provided==0.
 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#define HEADER  0xAAu
#define TRAILER 0x55u
#define MAX_LEN 128u

typedef enum {
    PR_OK = 0, PR_INCOMPLETE,
    PR_TOO_SHORT, PR_BAD_HEADER, PR_BAD_TRAILER, PR_BAD_LEN, PR_BAD_CRC
} pr_t;

typedef struct {
    uint8_t count, command, len, data[MAX_LEN];
} msg_t;

uint8_t crc8_maxim(const uint8_t *d, size_t n)
{
    uint8_t crc = 0u;
    for (size_t i = 0u; i < n; ++i) {
        crc ^= d[i];
        for (uint8_t b = 0u; b < 8u; ++b)
            crc = (crc & 1u) ? (uint8_t)((crc >> 1) ^ 0x8Cu) : (uint8_t)(crc >> 1);
    }
    return crc;
}

/* Whole packet in one buffer. Length comes from LEN, never from scanning
 * for 0x55 (DATA may contain it). out is filled only on PR_OK. */
pr_t parse_packet(const uint8_t *b, size_t n, msg_t *out)
{
    if (n < 6u)                  return PR_TOO_SHORT;
    if (b[0] != HEADER)          return PR_BAD_HEADER;
    uint8_t len = b[3];
    if (len > MAX_LEN)           return PR_BAD_LEN;
    if (n != (size_t)6u + len)   return PR_BAD_LEN;
    size_t crc_i = (size_t)4u + len;
    if (b[crc_i + 1u] != TRAILER) return PR_BAD_TRAILER;
    if ((crc8_maxim(&b[1], crc_i - 1u) ^ b[crc_i]) != 0u) return PR_BAD_CRC;

    out->count = b[1]; out->command = b[2]; out->len = len;
    memcpy(out->data, &b[4], len);
    return PR_OK;
}

static const uint8_t M1[] = {0xAA,0x21,0x30,0x04,0xDE,0xAD,0xBE,0xEF,0xA4,0x55};
static const uint8_t M2[] = {0xAA,0x22,0x31,0x01,0x7F,0x7F,0x55};
static const uint8_t M3[] = {0xAA,0x23,0x32,0x03,0x10,0x20,0x30,0x00,0x55}; /* CRC should be 0x55 -> bad on purpose */

static void show(const char *tag, pr_t st, const msg_t *m)
{
    if (st == PR_OK) {
        printf("%-9s OK  count=0x%02X cmd=0x%02X data=[", tag, m->count, m->command);
        for (uint8_t i = 0u; i < m->len; ++i)
            printf("%02X%s", m->data[i], i + 1u < m->len ? " " : "");
        printf("]\n");
    } else {
        printf("%-9s ERR (%d)\n", tag, (int)st);
    }
}

int main(void)
{
    msg_t m;
    show("Msg 1", parse_packet(M1, sizeof M1, &m), &m);
    show("Msg 2", parse_packet(M2, sizeof M2, &m), &m);
    show("Msg 3", parse_packet(M3, sizeof M3, &m), &m);
    return 0;
}
