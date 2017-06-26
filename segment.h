/*
 *            segment.h
 * written by Shinichi Awamoto, 2017
 *
 * セグメンテーション関連の定義
 */

#pragma once

#define KERNEL_CS (0x10)
#define KERNEL_DS (0x18)
#define TSS       (0x20)

#ifndef ASM_FILE

#include "common.h"

void gdt_init();

#endif // ASM_FILE
