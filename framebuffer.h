/*
 *            framebuffer.h
 * written by Shinichi Awamoto, 2017
 * 
 * 画面描画関連の関数の宣言
 */

#pragma once

#include "common.h"
#include "multiboot2.h"

void framebuffer_init(struct multiboot_tag_framebuffer_common *tag);
uint64_t framebuffer_ntimesput(uint64_t *hpet_addr);
void framebuffer_printf(char *fmt, ...);
