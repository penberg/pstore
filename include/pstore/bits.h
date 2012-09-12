#ifndef PSTORE_BITS_H
#define PSTORE_BITS_H

static inline bool has_zero_byte(unsigned int v)
{
    return ((v - 0x01010101UL) & ~v & 0x80808080UL);
}

#endif /* PSTORE_BITS_H */
