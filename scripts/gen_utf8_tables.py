#!/usr/bin/python

# This program was used to generate jv_utf8_tables.gen.h

mask = lambda n: (1 << n) - 1


def print_table(type, name, t):
    assert len(t) == 256
    print("static const %s %s[] =" % (type, name))
    first = True
    for i in range(0,len(t),16):
        print ((" {" if i == 0 else "  ") +
               ", ".join("0x%02x"%n for n in t[i:i+16]) + 
               ("," if i + 16 < 256 else "};"))


def utf8info(c):
    if c < 0x80: return 1, mask(7)
    if 0x80 <= c <= 0xBF: return 255, mask(6)
    if 0xC0 <= c <= 0xC1: return 0, 0
    if 0xC2 <= c <= 0xDF: return 2, mask(5)
    if 0xE0 <= c <= 0xEF: return 3, mask(4)
    if 0xF0 <= c <= 0xF4: return 4, mask(3)
    if 0xF4 <= c <= 0xFF: return 0, 0
        
table = lambda i: [utf8info(c)[i] for c in range(256)]

print("#define UTF8_CONTINUATION_BYTE ((unsigned char)255)")

print_table("unsigned char", "utf8_coding_length", table(0))
print_table("unsigned char", "utf8_coding_bits", table(1))
