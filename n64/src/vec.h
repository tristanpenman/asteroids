#ifndef __VEC_H
#define __VEC_H

struct vec_2d
{
    float x;
    float y;
};

/**
 * Unit vector (1.0, 1.0)
 */
extern const struct vec_2d vec_2d_unit;

/**
 * Zero vector (0.0, 0.0)
 */
extern const struct vec_2d vec_2d_zero;

/**
 * Calculate the dot product of a pair of 2D (float) vectors
 *
 * @param  u  pointer to first vector
 * @param  v  pointer to second vector
 */
float vec_2d_dot(const struct vec_2d *u, const struct vec_2d *v);

/**
 * Calculate the magnitude of a 2D (float) vector
 *
 * @param  v  pointer to vector
 */
float vec_2d_magnitude(const struct vec_2d *v);

/**
 * Normalise a 2D (float) vector in place
 *
 * @param  v  pointer to vector
 *
 * @return  initial magnitude of vector
 */
float vec_2d_normalise(struct vec_2d *v);

/**
 * Return a normalised 2D (float) vector
 *
 * @param  v  pointer to vector
 *
 * @return  normalised vector
 */
struct vec_2d vec_2d_normalised(const struct vec_2d *v);

/**
 * Scale a 2D (float) vector in place
 *
 * @param  v  pointer to vector
 * @param  f  scaling factor
 */
void vec_2d_scale(struct vec_2d *v, float f);

#endif
