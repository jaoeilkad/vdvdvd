
#ifndef _NX_H
#define _NX_H

#include "sdl/include/LRSDL.h"
#include <ctype.h>
#include "nx_logger.h"

#ifdef _WIN32
#include "libretro/msvc_compat.h"
#endif

#include "config.h"
#include "common/basics.h"
#include "common/BList.h"
#include "common/DBuffer.h"
#include "common/DString.h"
#include "common/InitList.h"

#include "graphics/graphics.h"
#include "graphics/tileset.h"
#include "graphics/sprites.h"
typedef SIFPoint	Point;
using Tileset::draw_tile;

#define CSF				9
class Object;

#include "trig.h"
#include "autogen/sprites.h"
#include "dirnames.h"
#include "TextBox/TextBox.h"
#include "graphics/font.h"

#include "input.h"
#include "tsc.h"
#include "stageboss.h"
#include "ai/ai.h"
#include "map.h"
#include "statusbar.h"
#include "floattext.h"
#include "object.h"
#include "ObjManager.h"
#include "game.h"
#include "caret.h"
#include "screeneffect.h"
#include "settings.h"
#include "slope.h"
#include "player.h"
#include "p_arms.h"

#include "sound/sound.h"

#endif
