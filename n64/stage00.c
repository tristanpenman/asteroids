#include <assert.h>
#include <math.h>
#include <nusys.h>
#include <stdlib.h>

#include "canvas.h"
#include "gfx.h"
#include "shape.h"
#include "vec.h"

#define BUFFER 5
#define SHIP_ACCELERATION 100.45f

extern NUContData contdata[1];
extern u8 contPattern;

extern const struct shape asteroid_shapes[];

static const float ratio = (float)SCREEN_HT / (float)SCREEN_WD;
static const float factor = 1.0f / 60.0f;
static const float degrees_to_radians = M_PI / 180.0f;

static const float max_x =  (float)SCREEN_WD / 2.0f + BUFFER;
static const float min_x = -(float)SCREEN_WD / 2.0f - BUFFER;

static const float max_y =  (float)SCREEN_HT / 2.0f + BUFFER;
static const float min_y = -(float)SCREEN_HT / 2.0f - BUFFER;

static float theta;

static float pos_x;
static float pos_y;

static float vel_x;
static float vel_y;

static uint8_t asteroid1;

#define MAX_VERTICES 64

static Vtx asteroid_vtx_data[MAX_VERTICES];

void draw(Dynamic* dynamicp);
void draw_asteroid(Dynamic* dynamicp);

void initStage00(void)
{
    canvas_reset();

    asteroid1 = canvas_load_shape(&asteroid_shapes[0]);

    theta = 0.0;

    pos_x = 0.0;
    pos_y = 0.0;

    vel_x = 0.0;
    vel_y = 0.0;
}

void makeDL00(void)
{
    char conbuf[20];

    canvas_start_drawing();

    draw(&gfx_dynamic);

    canvas_draw_triangles(asteroid1, vec_2d_zero, 0, vec_2d_zero);

    canvas_finish_drawing(false);

    // Change character representation positions */
    nuDebConTextPos(0,12,23);
    sprintf(conbuf, "pos_x=%5.1f", pos_x);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,12,24);
    sprintf(conbuf, "pos_y=%5.1f", pos_y);
    nuDebConCPuts(0, conbuf);

    // Display characters on the frame buffer
    nuDebConDisp(NU_SC_SWAPBUFFER);
}

void updateGame00(void)
{
    static float vel = 5.0;
    static float rot;

    // Data reading of controller 1
    nuContDataGetEx(contdata, 0);

    if (contdata[0].stick_x < -20) {
        theta -= vel;
    } else if (contdata[0].stick_x > 20) {
        theta += vel;
    }

    if (theta > 360.0) {
        theta -= 360.0;
    } else if (theta < 0.0) {
        theta += 360.0;
    }

    rot = theta * degrees_to_radians;

    if (contdata[0].button & B_BUTTON) {
        vel_x += sinf(rot) * SHIP_ACCELERATION * factor;
        vel_y -= cosf(rot) * SHIP_ACCELERATION * factor;
    }

    pos_x += vel_x * factor;
    pos_y += vel_y * factor;

    if (pos_y > max_y) {
        pos_y = min_y;
    } else if (pos_y < min_y) {
        pos_y = max_y;
    }

    if (pos_x > max_x) {
        pos_x = min_x;
    } else if (pos_x < min_x) {
        pos_x = max_x;
    }
}

void draw(Dynamic* dynamicp)
{
    static Vtx shade_vtx[] =  {
        {  0.00, -6.40, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        { -5.12,  5.76, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        {  5.12,  5.76, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        { -4.48,  4.00, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        {  4.48,  4.00, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
        {  0.00,  6.40, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff }
    };

    guOrtho(&(dynamicp->projection),
        0.0f,       // left
        SCREEN_WD,  // right
        SCREEN_HT,  // bottom
        0.0f,       // top
        1.0f,       // near
        10.0f,      // far
        1.0f);      // scale

    guTranslate(&(dynamicp->modeling),
        (float)SCREEN_WD / 2.0f + pos_x,
        (float)SCREEN_HT / 2.0f + pos_y,
        0.0f);
    guRotate(&(dynamicp->rotation), theta, 0.0f, 0.0f, 1.0f);
    // Global scaling factor
    // TODO: Could this be incorporated into projection matrix?
    guScale(&gfx_dynamic.scale, 1.0f, 1.2f, 1.f);

    // load projection matrix
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)),
        G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_PROJECTION);

    // load and transform model-view matrix
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->modeling)),
        G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->rotation)),
        G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->scale)),
        G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);

    // load vertices (I think this is limited to loading 32 vertices at once)
    gSPVertex(glistp++, &(shade_vtx[0]), 6, 0);

    // Synchronizes RDP attribute updates by waiting for pixels to be processed
    // during the rendering of primitives
    gDPPipeSync(glistp++);

    // number of pixels per cycle
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);

#ifdef LINES
    // support anti-aliased, translucent lines
    gDPSetRenderMode(glistp++, G_RM_AA_XLU_LINE, G_RM_AA_XLU_LINE2);
#else
    gDPSetRenderMode(glistp++, G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
#endif

    gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
    gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH);

#ifdef LINES
    gSPLineW3D(glistp++, 0, 1, 1.5, 0);
    gSPLineW3D(glistp++, 0, 2, 1.5, 0);
    gSPLineW3D(glistp++, 3, 4, 1.5, 0);
#else
    gSP1Triangle(glistp++, 0, 1, 2, 0);
#endif
}
