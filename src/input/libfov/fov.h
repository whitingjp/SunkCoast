/*
 * Copyright (C) 2006-2007, Greg McIntyre. All rights reserved. See the file
 * named COPYING in the distribution for more details.
 */

/**
 * \mainpage Field of View Library
 * 
 * \section about About
 * 
 * This is a C library which implements a course-grained lighting
 * algorithm suitable for tile-based games such as roguelikes.
 * 
 * \section copyright Copyright
 * 
 * \verbinclude COPYING
 * 
 * \section thanks Thanks
 * 
 * Thanks to Bj&ouml;rn Bergstr&ouml;m
 * <bjorn.bergstrom@hyperisland.se> for the algorithm.
 * 
 */

/**
 * \file fov.h
 * Field-of-view algorithm for dynamically casting light/shadow on a
 * low resolution 2D raster.
 */
#ifndef LIBFOV_HEADER
#define LIBFOV_HEADER

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Eight-way directions. */
typedef enum {
    FOV_EAST = 0,
    FOV_NORTHEAST,
    FOV_NORTH,
    FOV_NORTHWEST,
    FOV_WEST,
    FOV_SOUTHWEST,
    FOV_SOUTH,
    FOV_SOUTHEAST
} fov_direction_type;

/** Values for the shape setting. */
typedef enum {
    FOV_SHAPE_CIRCLE_PRECALCULATE,
    FOV_SHAPE_SQUARE,
    FOV_SHAPE_CIRCLE,
    FOV_SHAPE_OCTAGON
} fov_shape_type;

/** Values for the corner peek setting. */
typedef enum {
    FOV_CORNER_NOPEEK,
    FOV_CORNER_PEEK
} fov_corner_peek_type;

/** Values for the opaque apply setting. */
typedef enum {
    FOV_OPAQUE_APPLY,
    FOV_OPAQUE_NOAPPLY
} fov_opaque_apply_type;

/** @cond INTERNAL */
typedef /*@null@*/ unsigned *height_array_t;
/** @endcond */

typedef struct {
    /** Opacity test callback. */
    /*@null@*/ bool (*opaque)(void *map, int x, int y);

    /** Lighting callback to set lighting on a map tile. */
    /*@null@*/ void (*apply)(void *map, int x, int y, int dx, int dy, void *src);

    /** Shape setting. */
    fov_shape_type shape;

    /** Whether to peek around corners. */
    fov_corner_peek_type corner_peek;

    /** Whether to call apply on opaque tiles. */
    fov_opaque_apply_type opaque_apply;

    /** \cond INTERNAL */

    /** Pre-calculated data. \internal */
    /*@null@*/ height_array_t *heights;

    /** Size of pre-calculated data. \internal */
    unsigned numheights;

    /** \endcond */
} fov_settings_type;

/** The opposite direction to that given. */
#define fov_direction_opposite(direction) ((fov_direction_type)(((direction)+4)&0x7))

/**
 * Set all the default options. You must call this option when you
 * create a new settings data structure.
 *
 * These settings are the defaults used:
 *
 * - shape: FOV_SHAPE_CIRCLE_PRECALCULATE
 * - corner_peek: FOV_CORNER_NOPEEK
 * - opaque_apply: FOV_OPAQUE_APPLY
 *
 * Callbacks still need to be set up after calling this function.
 *
 * \param settings Pointer to data structure containing settings.
 */
void fov_settings_init(fov_settings_type *settings);

/**
 * Set the shape of the field of view.
 *
 * \param settings Pointer to data structure containing settings.
 * \param value One of the following values, where R is the radius:
 *
 * - FOV_SHAPE_CIRCLE_PRECALCULATE \b (default): Limit the FOV to a
 * circle with radius R by precalculating, which consumes more memory
 * at the rate of 4*(R+2) bytes per R used in calls to fov_circle. 
 * Each radius is only calculated once so that it can be used again. 
 * Use fov_free() to free this precalculated data's memory.
 *
 * - FOV_SHAPE_CIRCLE: Limit the FOV to a circle with radius R by
 * calculating on-the-fly.
 *
 * - FOV_SHAPE_OCTOGON: Limit the FOV to an octogon with maximum radius R.
 *
 * - FOV_SHAPE_SQUARE: Limit the FOV to an R*R square.
 */
void fov_settings_set_shape(fov_settings_type *settings, fov_shape_type value);

/**
 * <em>NOT YET IMPLEMENTED</em>.
 *
 * Set whether sources will peek around corners.
 *
 * \param settings Pointer to data structure containing settings.
 * \param value One of the following values:
 *
 * - FOV_CORNER_PEEK \b (default): Renders:
\verbatim
  ........
  ........
  ........
  ..@#    
  ...#    
\endverbatim
 * - FOV_CORNER_NOPEEK: Renders:
\verbatim
  ......
  .....
  ....
  ..@#
  ...#
\endverbatim
 */
void fov_settings_set_corner_peek(fov_settings_type *settings, fov_corner_peek_type value);

/**
 * Whether to call the apply callback on opaque tiles.
 *
 * \param settings Pointer to data structure containing settings.
 * \param value One of the following values:
 *
 * - FOV_OPAQUE_APPLY \b (default): Call apply callback on opaque tiles.
 * - FOV_OPAQUE_NOAPPLY: Do not call the apply callback on opaque tiles.
 */
void fov_settings_set_opaque_apply(fov_settings_type *settings, fov_opaque_apply_type value);

/**
 * Set the function used to test whether a map tile is opaque.
 *
 * \param settings Pointer to data structure containing settings.
 * \param f The function called to test whether a map tile is opaque.
 */
void fov_settings_set_opacity_test_function(fov_settings_type *settings, bool (*f)(void *map, int x, int y));

/**
 * Set the function used to apply lighting to a map tile.
 *
 * \param settings Pointer to data structure containing settings.
 * \param f The function called to apply lighting to a map tile.
 */
void fov_settings_set_apply_lighting_function(fov_settings_type *settings, void (*f)(void *map, int x, int y, int dx, int dy, void *src));

/**
 * Free any memory that may have been cached in the settings
 * structure.
 *
 * \param settings Pointer to data structure containing settings.
 */
void fov_settings_free(fov_settings_type *settings);

/**
 * Calculate a full circle field of view from a source at (x,y).
 *
 * \param settings Pointer to data structure containing settings.
 * \param map Pointer to map data structure to be passed to callbacks.
 * \param source Pointer to data structure holding source of light.
 * \param source_x x-axis coordinate from which to start.
 * \param source_y y-axis coordinate from which to start.
 * \param radius Euclidean distance from (x,y) after which to stop.
 */
void fov_circle(fov_settings_type *settings, void *map, void *source,
                int source_x, int source_y, unsigned radius
);

/**
 * Calculate a field of view from source at (x,y), pointing
 * in the given direction and with the given angle. The larger
 * the angle, the wider, "less focused" the beam. Each side of the
 * line pointing in the direction from the source will be half the
 * angle given such that the angle specified will be represented on
 * the raster.
 *
 * \param settings Pointer to data structure containing settings.
 * \param map Pointer to map data structure to be passed to callbacks.
 * \param source Pointer to data structure holding source of light.
 * \param source_x x-axis coordinate from which to start.
 * \param source_y y-axis coordinate from which to start.
 * \param radius Euclidean distance from (x,y) after which to stop.
 * \param direction One of eight directions the beam of light can point.
 * \param angle The angle at the base of the beam of light, in degrees.
 */
void fov_beam(fov_settings_type *settings, void *map, void *source,
              int source_x, int source_y, unsigned radius,
              fov_direction_type direction, float angle
);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
