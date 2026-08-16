#ifndef ASTEROIDS_GAME_TEXT_H
#define ASTEROIDS_GAME_TEXT_H

/**
 * Invalidate cached glyph shape handles after the canvas is reset.
 */
void text_reset(void);

/**
 * Draw text at the specified position.
 */
void text_draw(const char *text, float x, float y, float scale_factor);

/**
 * Draw horizontally centered text.
 */
void text_draw_centered(const char *text, float y, float scale_factor);

#endif
