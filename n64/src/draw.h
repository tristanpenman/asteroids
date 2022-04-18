#ifndef __DRAW_H
#define __DRAW_H

void draw_text_ex(const char *s, float size, float x, float y, float spacing);
void draw_text(const char *s, float size, float x, float y);
void draw_text_centered_ex(const char *s, float size, float y, float spacing);
void draw_text_centered(const char *s, float size, float y);

#endif
