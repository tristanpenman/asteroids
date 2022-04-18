#include <string.h>

#include "canvas.h"
#include "draw.h"
#include "defines.h"

void draw_text_centered(const char *s, float size, float y)
{
    const float width = ((float) strlen(s) * (FONT_WIDTH + FONT_SPACE)) - FONT_SPACE;

    canvas_draw_text(s, 0 - (width * size / 2.0f), y, FONT_SPACE, size);
}
