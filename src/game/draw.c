#include "draw.h"

#if defined(ASTEROIDS_PLATFORM_N64)

void draw_explosions(const struct explosion *ee, unsigned int n)
{
    (void)ee;
    (void)n;
}

void draw_level_title(int level)
{
    (void)level;
}

void draw_lives(int lives)
{
    (void)lives;
}

void draw_player_exploding(const struct player *p)
{
    (void)p;
}

void draw_score(int score)
{
    (void)score;
}

#else

#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#include <SDL.h>
#endif

#include <SDL_opengl.h>

#include "canvas.h"
#include "defines.h"
#include "entities.h"
#include "highscores.h"
#include "mathdefs.h"
#include "options.h"
#include "text.h"
#include "vec.h"

const struct vec_2d origin = {
    1.0f / 2.0f,
    ((GLfloat)LOGICAL_HEIGHT_PX / (GLfloat)LOGICAL_WIDTH_PX) / 2.0f
};

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static void gl_vertex_2f(float x, float y)
{
    glVertex3f(x, y, 0.f);
}

static void draw_ship(float c)
{
    glColor3f((GLfloat) c, (GLfloat) c, (GLfloat) c);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(0.0f, 0 - SHIP_PIVOT, 0.0f);

    glBegin(GL_LINE_STRIP);
    gl_vertex_2f(-0.012f,  0.038f);
    gl_vertex_2f( 0.000f,  0.000f);
    gl_vertex_2f( 0.012f,  0.038f);
    glEnd();

    glBegin(GL_LINES);
    gl_vertex_2f(-0.0105f,  0.0325f);
    gl_vertex_2f( 0.0105f,  0.0325f);
    glEnd();

    glPopMatrix();

    glColor3f(1.f, 1.f, 1.f);
}

static void draw_ship_explosion(const struct player *p)
{
    float d;
    float t = p->death_delay;

    GLfloat c = (GLfloat)(1.0f - (t / SHIP_DEATH_DELAY));

    for (unsigned int i = 0; i < SHIP_EXPLOSION_SHARDS; i++) {
        d = p->shards[i].angle;
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(
            (GLfloat)(sinf(d) * t * SHIP_EXPLOSION_SPEED),
            (GLfloat)(0 - cosf(d) * t * SHIP_EXPLOSION_SPEED), 0.0f);
        glRotatef((GLfloat)(p->shards[i].rot * RAD_TO_DEG), 0.0f, 0.0f, 1.0f);

        glColor3f(c, c, c);
        glBegin(GL_LINES);
        gl_vertex_2f(-0.008f, 0.0f);
        gl_vertex_2f(0.008f, 0.0f);
        glEnd();

        glPopMatrix();
    }

    glColor3f(1.f, 1.f, 1.f);
}

static void draw_text_centered_ex(const char *s, GLfloat size, GLfloat y, GLfloat spacing)
{
    const GLfloat width = ((GLfloat) strlen(s) * (FONT_WIDTH + spacing)) - spacing;

    text_draw(s, 0 - (width * size / 2.0f), y, size);
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

void draw_explosions(const struct explosion *ee, unsigned int n)
{
    float a;

    for (unsigned int i = 0; i < n; i++, ee++) {
        if (ee->visible == true) {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glTranslatef((GLfloat)(origin.x + ee->pos.x), (GLfloat)(origin.y + ee->pos.y), 0.0f);
            glScalef(EXPLOSION_SPEED, EXPLOSION_SPEED, 1.0f);
            glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_POINTS);
            for (unsigned int p = 0; p < EXPLOSION_PARTICLES; p++) {
                a = (float)(2 * M_PI) / (float) EXPLOSION_PARTICLES * (float) p;
                gl_vertex_2f(
                    (GLfloat)(sinf(a) * ee->time),
                    (GLfloat)(0 - cosf(a) * ee->time));
            }
            glEnd();
            glPopMatrix();
        }
    }
}

void draw_level_title(int level)
{
    char titlecard[100];
    snprintf(titlecard, 100, "LEVEL %u", level);
    draw_text_centered(titlecard, 0.35f, -0.05f);
}

void draw_lives(int lives)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(0.5f, 0.5f, 1.0f);
    glTranslatef(0.05f, 0.11f, 0.0f);
    for (int i = 0; i < lives; i++) {
        glTranslatef(0.04f, 0.0f, 0.0f);
        draw_ship(1.0f);
    }

    glPopMatrix();
}

void draw_player_exploding(const struct player *p)
{
    const float c = 1.0f - (p->death_delay / SHIP_EXPLOSION_LENGTH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef((GLfloat)(origin.x + p->pos.x), (GLfloat)(origin.y + p->pos.y), 0.f);
    glRotatef((GLfloat)(p->rot * RAD_TO_DEG), 0.f, 0.f, 1.f);
    draw_ship(c > 1.f ? 1.f : c);
    draw_ship_explosion(p);
    glPopMatrix();
}

void draw_score(int score)
{
    static char buffer[SCORE_BUFFER_SIZE];

    sprintf(buffer, "%u", score);
    text_draw(buffer, -0.475f, -0.365f, 0.35f);
}

#endif
