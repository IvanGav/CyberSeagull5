#pragma once
#include "drillengine/DrillLib.h"

namespace Cyber5eagull {
extern U32 activeEditingLayer;
extern U32 totalCyberSeagullsOnShip;
extern const U32 TARGET_CYBERSEAGULL_COUNT;
extern F32 shipOffset;
extern B32 gameOver;
struct Texture;
extern Resources::Texture* currentTooltip;
extern V2I tooltipTopLeft;
const I32 TOOLTIP_SCALE = 2;
}