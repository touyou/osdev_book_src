#!/usr/bin/env python

from sys import stdin
import string
import commands


def lineToBinary(line):
    rest = 8 - len(line)
    if rest > 0:
        line += ' ' * rest
    line = line.translate(string.maketrans('# ', '10'))
    return '0b' + line + ',\n'


def bannerToCCode(banner):
    bits = [lineToBinary(line.rstrip()) for line in banner.split('\n')]
    font = '{\n' + ''.join(bits).rstrip(',\n') + '\n}'
    return font


def nullbanner():
    return '{}'


def asciiToBannerBInary(ascii):
    if ascii == 0:
        return nullbanner() + '\n,'
    banner = commands.getoutput('banner \"%c\"' % chr(ascii))
    if len(banner) < 10:
        return nullbanner() + '\n,'
    else:
        return bannerToCCode(banner) + '\n,'


if __name__ == '__main__':
    font = [asciiToBannerBInary(i) for i in range(0,128)]
    font = 'uint8_t font[128][8] = {\n' + ''.join(font).rstrip(',') + '};\n'
    f = open('font.c', 'w')
    f.write(font)
    f.close()
