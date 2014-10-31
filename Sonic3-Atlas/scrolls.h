#ifndef SCROLLS_H
#define SCROLLS_H
#pragma once

#include "main.h"

#define HSCROLL (PAL+0x80)
#define HSIZE ((BYTE*)(PAL+0x80+0x1000))

void UpdatePalAndScroll();

#endif
