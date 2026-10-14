#pragma once
#include "Game.h"
namespace view {
void text(int x,int y,const char *s,uint8_t color);
void center(int y,const char *s,uint8_t color);
void number(int x,int y,int32_t n,uint8_t color);
void cash(int x,int y,uint32_t n,uint8_t color);
void time(int x,int y,uint16_t ticks,uint8_t color);
void header(const char *title,uint32_t money);
void footer(const char *hint);
void row(int y,const char *label,bool active,uint8_t color);
void city(int32_t distance);
void car(int x,int y,uint8_t color,bool boost,bool slip,uint16_t tick);
void raceView(const vr::Race &r);
void title();
}
