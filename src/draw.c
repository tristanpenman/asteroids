#include <math.h>
#include <stdio.h>
#include <string.h>

#include <SDL_opengl.h>

#include "options.h"
#include "types.h"

#define RAD_TO_DEG (180.0 / M_PI)

extern float pixel_density;
extern GLfloat *_asteroids[];
extern GLfloat *_font[];

const struct vec_2d origin = {1.0f / 2.0f,
    ((GLfloat)LOGICAL_HEIGHT_PX / (GLfloat)LOGICAL_WIDTH_PX) / 2.0f};

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

void glVertex_2f(float x, float y)
{
    glVertex3f(x, y, 0.f);
}

static void draw_colon()
{
    glBegin(GL_LINE_LOOP);
    glVertex_2f(.0125f, .0125f);
    glVertex_2f(.0175f, .0125f);
    glVertex_2f(.0175f, .0175f);
    glVertex_2f(.0125f, .0175f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex_2f(.0125f, .0425f);
    glVertex_2f(.0175f, .0425f);
    glVertex_2f(.0175f, .0475f);
    glVertex_2f(.0125f, .0475f);
    glEnd();
}

static void draw_ship(bool thruster, float c)
{
    glColor3f((GLfloat) c, (GLfloat) c, (GLfloat) c);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(0.0f, 0 - SHIP_PIVOT, 0.0f);

    glBegin(GL_LINE_STRIP);
    glVertex_2f(-0.012f,  0.038f);
    glVertex_2f( 0.000f,  0.000f);
    glVertex_2f( 0.012f,  0.038f);
    glEnd();

    glBegin(GL_LINES);
    glVertex_2f(-0.0105f,  0.0325f);
    glVertex_2f( 0.0105f,  0.0325f);
    glEnd();

    if (true == thruster) {
        glBegin(GL_LINE_STRIP);
        glVertex_2f(-0.006f, 0.0325f);
        glVertex_2f( 0.000f, 0.0400f);
        glVertex_2f( 0.006f, 0.0325f);
        glEnd();
    }

    glPopMatrix();
}

static void draw_ship_explosion(const struct player *p)
{
    unsigned int i;

    float d;
    float t = p->death_delay;

    GLfloat c = (GLfloat)(1.0f - (t / SHIP_DEATH_DELAY));

    for (i = 0; i < SHIP_EXPLOSION_SHARDS; i++) {
        d = p->shards[i].angle;
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(
            (GLfloat)(sinf(d) * t * SHIP_EXPLOSION_SPEED),
            (GLfloat)(0 - cosf(d) * t * SHIP_EXPLOSION_SPEED), 0.0f);
        glRotatef((GLfloat)(p->shards[i].rot * RAD_TO_DEG), 0.0f, 0.0f, 1.0f);

        glColor3f(c, c, c);
        glBegin(GL_LINES);
          glVertex_2f(-0.008f, 0.0f);
          glVertex_2f(0.008f, 0.0f);
        glEnd();

        glPopMatrix();
    }
}

static void draw_text_ex(const char *s, GLfloat size, GLfloat x, GLfloat y, float spacing)
{
    const GLfloat *g = NULL;

    glEnableClientState(GL_VERTEX_ARRAY);
    glColor3f(1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(x, y, 0.0f);

    x = 0.0f;

    while (*s) {
        glPushMatrix();
        glScalef(size, size, 1.0f);
        glTranslatef(x, 0.0f, 0.0f);

        if (*s == ':') {
            draw_colon();
        } else {
            g = _font[(int) *s];  /* First element of each glyph is vertex count */
            if (NULL != g) {
                glVertexPointer(2, GL_FLOAT, 0, g + 1);
                glDrawArrays(GL_LINE_STRIP, 0, (unsigned int) g[0]);
            }
        }

        glPopMatrix();

        x += FONT_WIDTH;
        x += spacing;
        s++;
    }

    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
}

static void draw_text(const char *s, GLfloat size, GLfloat x, GLfloat y)
{
    draw_text_ex(s, size, x, y, FONT_SPACE);
}

static void draw_text_centered_ex(const char *s, GLfloat size, GLfloat y, float spacing)
{
    const GLfloat width = ((GLfloat) strlen(s) * (FONT_WIDTH + spacing)) - spacing;

    draw_text_ex(s, size, (1.0f / 2.0f) - (width * size / 2.0f), y, spacing);
}

static void draw_text_centered(const char *s, GLfloat size, GLfloat y)
{
    draw_text_centered_ex(s, size, y, FONT_SPACE);
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void draw_asteroids(const struct asteroid *aa, unsigned int n, bool dark, float residual)
{
    unsigned int i;

    const GLfloat *v;

    GLfloat tx, ty;
    GLfloat rd;

    if (dark == true) {
        glColor3f(0.4f, 0.4f, 0.4f);
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    glEnableClientState(GL_VERTEX_ARRAY);

    for (i = 0; i < n; i++, aa++) {
        if (true == aa->visible) {
            tx = (GLfloat) (origin.x + aa->pos.x + aa->vel.x * residual);
            ty = (GLfloat) (origin.y + aa->pos.y + aa->vel.y * residual);
            rd = (GLfloat) (aa->rot * RAD_TO_DEG);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();

            glTranslatef(tx, ty, 0.0f);
            glRotatef(rd, 0.0f, 0.0f, 1.0f);

            glScalef(1.0f / (GLfloat) aa->size, 1.0f / (GLfloat) aa->size, 1.0f);

            v = _asteroids[aa->shape];    /* First element of each asteroid is vertex count */

            glVertexPointer(2, GL_FLOAT, 0, v + 1);
            glDrawArrays(GL_LINE_LOOP, 0, (unsigned int) v[0]);

            glPopMatrix();
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);
}

void draw_bullets(const struct bullet *bb, unsigned int n)
{
    unsigned int i;
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef((GLfloat) origin.x, (GLfloat) origin.y, 0.0f);
    glColor3f(1.f, 1.f, 1.f);
    glBegin(GL_LINES);
    for (i = 0; i < n; i++) {
        if (true == bb[i].visible) {
            const float dx = bb[i].vel.x / 300.f;
            const float dy = bb[i].vel.y / 300.f;
            glVertex_2f((GLfloat)(bb[i].pos.x + dx), (GLfloat)(bb[i].pos.y + dy));
            glVertex_2f((GLfloat)(bb[i].pos.x - dx), (GLfloat)(bb[i].pos.y - dy));
        }
    }
    glEnd();
    glPopMatrix();
}

void draw_explosions(const struct explosion *ee, unsigned int n)
{
    unsigned int i = 0;
    unsigned int p = 0;
    float a;
    for (i = 0; i < n; i++, ee++) {
        if (ee->visible == true) {
            //glPointSize(pixel_density);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glTranslatef((GLfloat)(origin.x + ee->pos.x), (GLfloat)(origin.y + ee->pos.y), 0.0f);
            glScalef(EXPLOSION_SPEED, EXPLOSION_SPEED, 1.0f);
            glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_POINTS);
            for (p = 0; p < EXPLOSION_PARTICLES; p++) {
                a = ((2 * M_PI) / (float) EXPLOSION_PARTICLES) * (float) p;
                glVertex_2f((GLfloat)(sinf(a) * ee->time), (GLfloat)(0 - cosf(a) * ee->time));
            }
            glEnd();
            glPopMatrix();
        }
    }
}

void draw_gameover_text()
{
    draw_text_centered("GAME OVER", 0.39f, 0.12f);
}

void draw_highscores(const struct highscores *scores)
{
    int i;
    char str[100];

    draw_text_centered("HIGH SCORES", 0.39f, 0.12f);

    for (i = 0; i < 10; i++) {
        if (scores->entries[i].used) {
            sprintf(str, "%2d   %.3s %10d ", i + 1, scores->entries[i].initials, scores->entries[i].score);
        } else {
            sprintf(str, "%2d   ---          - ", i + 1);
        }
        draw_text_centered(str, 0.24f, 0.21f + 0.03f * (float) i);
    }

    draw_text_centered("PRESS ENTER FOR MAIN MENU", 0.24f, 0.59f);
}

void draw_instructions()
{
    draw_text_centered("PRESS ENTER TO PLAY", 0.3f, 0.30f);
    draw_text_centered("SPACE - FIRE", 0.20f, 0.385f);
    draw_text_centered("ARROWS - DIRECTION", 0.20f, .415f);
    draw_text_centered("UP - THRUSTER", 0.20f, .445f);
#ifndef __EMSCRIPTEN__
    draw_text_centered("ESC - EXIT", 0.20f, .475f);
    draw_text_centered("PRESS H FOR HIGH SCORES", 0.24f, 0.555f);
#endif
}

void draw_level_title(unsigned int level)
{
    char titlecard[100];
    snprintf(titlecard, 100, "LEVEL %u", level);
    draw_text_centered(titlecard, 0.35f, .30f);
}

void draw_lives(int lives)
{
    int i;
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(0.5f, 0.5f, 1.0f);
    glTranslatef(0.05f, 0.11f, 0.0f);
    for (i = 0; i < lives; i++) {
        glTranslatef(0.04f, 0.0f, 0.0f);
        draw_ship(false, 1.0f);
    }

    glPopMatrix();
}

void draw_new_high_score_input(const char initials[4])
{
    draw_text_centered_ex(initials, 0.38f, 0.38f, FONT_SPACE * 10.f);
}

void draw_new_high_score_message()
{
    draw_text_centered("YOUR SCORE IS ONE OF THE TEN BEST", 0.25f, 0.25f);
    draw_text_centered("PLEASE ENTER YOUR INITIALS", 0.25f, 0.285f);
}

void draw_new_high_score_enter_to_continue()
{
    draw_text_centered("PRESS ENTER TO CONTINUE", 0.25f, 0.5f);
    draw_text_centered("OR BACKSPACE TO MAKE CHANGES", 0.25f, 0.535f);
}

void draw_player(const struct player *p)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef((GLfloat)(origin.x + p->pos.x), (GLfloat)(origin.y + p->pos.y), 0.f);
    glRotatef((GLfloat)(p->rot * RAD_TO_DEG), 0.f, 0.f, 1.f);
    draw_ship(p->keys.up == KS_ACTIVE && p->phase > SHIP_THRUSTER_BLINK ? true : false, 1.f);
    glPopMatrix();
}

void draw_player_exploding(const struct player *p)
{
    const float c = 1.0f - (p->death_delay / SHIP_EXPLOSION_LENGTH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef((GLfloat)(origin.x + p->pos.x), (GLfloat)(origin.y + p->pos.y), 0.f);
    glRotatef((GLfloat)(p->rot * RAD_TO_DEG), 0.f, 0.f, 1.f);
    draw_ship(false, c > 1.f ? 1.f : c);
    draw_ship_explosion(p);
    glPopMatrix();
}

void draw_score(unsigned int score)
{
    static char buffer[SCORE_BUFFER_SIZE];
    const unsigned int size = (unsigned int) floorf(1.0f + log10f((float) score));
    if (SCORE_BUFFER_SIZE > size) {
        snprintf(buffer, SCORE_BUFFER_SIZE, "%u", score);
    } else {
        snprintf(buffer, SCORE_BUFFER_SIZE, "ERROR");
    }

    draw_text(buffer, 0.35f, 0.025f, 0.008f);
}

void draw_title()
{
    draw_text_centered("ASTEROIDS", 0.8f, 0.17f);
}
