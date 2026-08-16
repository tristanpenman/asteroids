#include <string.h>

#include "canvas.h"
#include "data.h"
#include "defines.h"
#include "text.h"
#include "vec.h"

#define MAX_GLYPHS 128

static int font_shape_ids[MAX_GLYPHS];

void text_reset(void)
{
    for (int i = 0; i < MAX_GLYPHS; ++i) {
        font_shape_ids[i] = CANVAS_INVALID_SHAPE;
    }
}

void text_draw(const char *text, float x, float y, float scale_factor)
{
    struct vec_2d position = {x, y};
    const struct vec_2d scale = {scale_factor, scale_factor};

    while (*text != '\0') {
        int c = (unsigned char)*text;

        if (c < MAX_GLYPHS && ascii_to_font_mapping[c] >= 0) {
            if (font_shape_ids[c] == CANVAS_INVALID_SHAPE) {
                font_shape_ids[c] =
                    canvas_load_shape(&font_shape_data[ascii_to_font_mapping[c]]);
            }
            if (font_shape_ids[c] != CANVAS_INVALID_SHAPE) {
                canvas_draw_shape(font_shape_ids[c], position, 0.0f, scale);
            }
        }

        position.x += (FONT_WIDTH + FONT_SPACE) * scale_factor;
        ++text;
    }
}

void text_draw_centered(const char *text, float y, float scale_factor)
{
    const float width =
        ((float)strlen(text) * (FONT_WIDTH + FONT_SPACE)) - FONT_SPACE;

    text_draw(text, -(width * scale_factor / 2.0f), y, scale_factor);
}
