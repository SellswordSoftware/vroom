#pragma once
#include "Appearance.h"
namespace view {
void car(int x,int y,const look::Style &style,bool player,bool boosting,
         bool slip,uint16_t wheelTick,uint8_t number=0);
void playerCar(int x,int y,bool boosting=false,bool slip=false,uint16_t tick=0);
}
