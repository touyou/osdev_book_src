/*
 *            framebuffer.h
 * written by Shinichi Awamoto, 2017
 * 
 * 画面描画関連の関数の宣言
 */

#pragma once

struct multiboot_tag_framebuffer_common;

void framebuffer_init(struct multiboot_tag_framebuffer_common *tag);
void framebuffer_printf(char *fmt, ...);
