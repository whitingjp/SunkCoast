/* 

$Id

Copyright (C) 2009 Alexios Chouchoulas

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

*/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "astar.h"


///////////////////////////////////////////////////////////////////////////////
//
// DEBUGGING
//
///////////////////////////////////////////////////////////////////////////////

#ifdef HEAP_DEBUG
extern FILE * __heap_debugfp;
#endif // HEAP_DEBUG

#ifdef ASTAR_DEBUG
FILE * __astar_debugfp = NULL;

#  define __debug(format, ...) \
        assert (__astar_debugfp != NULL); \
        fprintf(__astar_debugfp, format, ##__VA_ARGS__);

#  ifdef SQUARE_HAS_OFS
#    define __debug_square(as, s) \
        __debug("(%d,%d) ofs=%u: f=%u, g=%u, h=%d, o=%d, c=%d\n",       \
                s->ofs % as->w, s->ofs / as->w,                         \
                s->ofs, s->f, s->g, s->h, s->open, s->closed)
#  else
#    define __debug_square(as, s) \
        __debug("ofs=%u: f=%u, g=%u, h=%d, o=%d, c=%d\n",               \
                s->ofs, s->f, s->g, s->h,               \
                s->open, s->closed)

#  endif // SQUARE_HAS_OFS

#else
#  define __debug(...)
#  define __debug_square(as, s)
#endif // ASTAR_DEBUG


///////////////////////////////////////////////////////////////////////////////
//
// DIRECTIONS
//
///////////////////////////////////////////////////////////////////////////////


/*
 * These are (dx,dy) values to go from the current square to the square's
 * child (next move).
 */

#define CC 10 // Cardinal direction cost
#define CD 14 // Diagonal direction cost int(10*sqrt(2))

//                                N    NE    E    SE     S    SW     W    NW
static const int32_t _dx[8] = {   0,    1,   1,    1,    0,   -1,   -1,   -1 };
static const int32_t _dy[8] = {  -1,   -1,   0,    1,    1,    1,    0,   -1 };
static const int32_t _mc[8] = {  CC,   CD,  CC,   CD,   CC,   CD,   CC,   CD };
#if defined(ASTAR_DEBUG) || defined(TEST_ASTAR)
static const char * dirs[8] = { "N", "\'", "E",  ".",  "S",  "/",  "W",  "`" };
static const char *names[8] = { "N", "NE", "E", "SE",  "S", "SW",  "W", "NW" };
#endif // defined(ASTAR_DEBUG) || defined(TEST_ASTAR)

// Reverse a direction. Funny how simple this is.
#define REVERSE_DIR(x) ((x) ^ 4)

// This is the base cost used in the heuristic function.
#define _HEURISTIC_FACTOR ((CC) - 2)

// The penalty incurred by changing direction.
#define _STEER_PENALTY ((CC) - 5)



///////////////////////////////////////////////////////////////////////////////
//
// CONVENIENCE MACROS
//
///////////////////////////////////////////////////////////////////////////////


// We use this a lot.
#define check_null(p,err) \
        if ((p) == NULL) {    \
                perror (err); \
                exit (EXIT_FAILURE); \
        }

// Calculate a grid offset given x, y and the grid's width (pitch).
#define mkofs(as, x, y) ((y) * ((as)->w) + (x))

#ifdef SQUARE_HAS_OFS
// Use the embedded offset field.
#define getofs(as, s) ((s)->ofs)
#else
// Use pointer arithmetic
#define getofs(as, s) ((s) - ((as)->grid))
#endif // SQUARE_HAS_OFS

#define getx(as, s) (getofs((as),(s)) % (as)->w)
#define gety(as, s) (getofs((as),(s)) / (as)->w)

// Used as return astar_error (as, error_code) to stop processing when
// an error occurs. It updates statistics.
#define astar_error(as, err) \
        (((as)->result=(err)),                                          \
         ((as)->str_result=#err),                                           \
         as->usecs = get_time_difference (&as->t0),                     \
         (err))

#define set_result(as,err) ((as)->result = err, (as)->str_result = #err)

// We use this to initialise a square_t payload.
#define __get_square(as, s, x, y)                                 \
        s->cost = (*as->get)(as->origin_x + x, as->origin_y + y); \
        s->g = 0;                                                 \
        s->h = 0;                                                 \
        s->f = 0;                                                 \
        s->open = 0;                                              \
        s->closed = 0;                                            \
        s->route = 0;                                             \
        s->init = 1;



///////////////////////////////////////////////////////////////////////////////
//
// DEFAULT HEURISTIC DISTANCE (MANHATTAN DISTANCE)
//
///////////////////////////////////////////////////////////////////////////////

/*
 * This is simple and fast, and works remarkably well as a first
 * (inadmissible) approximation of the path cost. This is the
 * Manhattan or City Block distance: on a city grid, all paths from A
 * to B are the same distance. Provided you keep moving towards B and
 * never away from it, it doesn't matter whether you turn by 90
 * degrees every block. For A(xa,ya) and B(xb,yb), the distance then
 * becomes | xb - xa | + | yb - ya |.
 *
 * This function is used if the caller doesn't supply one to provide a
 * better approximation. In practice, Manhattan Distance works very
 * well, though (and is also very cheap computationally, avoiding
 * square roots, which Euclidean distance doesn't).
 *
 */

static uint32_t 
manhattan_distance (const uint32_t x0, const uint32_t y0,
                    const uint32_t x1, const uint32_t y1)
{
        // Return the Manhattan distance between the two sets of
        // co-ordinates.

        return (uint32_t) abs ((int32_t) x1 - (int32_t) x0) +
                (uint32_t) abs ((int32_t) y1 - (int32_t) y0);
}


///////////////////////////////////////////////////////////////////////////////
//
// INTERNAL USE ONLY
//
///////////////////////////////////////////////////////////////////////////////


static void
astar_reset_grid (astar_t * as)
{
        // Don't reset it if it's already clean.
        if (as->grid_clean) return;
        __debug ("Resetting grid...\n");
        uint32_t i = 0, area = as->w * as->h;
        for (i = 0; i < area; i++) {
                as->grid[i].init = 0;
                as->grid[i].route = 0;
        }
}


static inline square_t *
get_square (astar_t * as, const uint32_t ofs, const uint32_t x, const uint32_t y)
{
        assert (as != NULL);
        register square_t * s = &(as->grid[ofs]);
        if (!s->init) {
                __get_square(as, s, x, y);
                assert (s->init);

#ifdef SQUARE_HAS_OFS
                s->ofs = ofs;
#endif // SQUARE_HAS_OFS

                as->gets++;
        }

#ifdef ASTAR_DEBUG
        assert (ofs == mkofs (as, x, y));
#ifdef SQUARE_HAS_OFS
        assert (s->ofs == mkofs (as, x, y));
#endif // SQUARE_HAS_OFS
        //__debug ("Got square: ");
        //__debug_square (as, s);
#endif // TEST_ASTAR

        return s;
}


static uint32_t
astar_find_best_compromise (astar_t * as)
{
        // All the work is done incrementally in astar_add_closed(). Calculate the last
        // bits.
        as->bestx = as->bestofs % as->w;
        as->besty = as->bestofs / as->w;
        return as->bestofs;
}


inline static void
astar_add_open (astar_t * as, square_t * s, uint32_t gridofs, uint32_t g, uint32_t h)
{
        assert (as != NULL);
        assert (&(as->grid[gridofs]) == s);
        assert (s->init);
        assert (!s->open);

#ifdef TEST_ASTAR
        //assert (g < 1000000);
#endif // TEST_ASTAR

        // As per A* algorithm.
        uint32_t f = g + h;
        
        __debug("\t+O Adding (%d,%d) (ofs=%d, f=%u, g=%u, h=%u) to open list.\n",
                gridofs % as->w, gridofs / as->w, gridofs, f, g, h);

        // Set values.
        s->f = f;
        s->g = g;
        s->h = h;
        s->open = 1;

        // Add the F value and square to the heap. Keep the heap offset.
        astar_heap_add (as->heap, f, s);
        as->open++;

        //__debug("++ Added (%d,%d) (ofs=%d, f=%u, g=%u, h=%u) to open list.\n",
        //      gridofs % as->w, gridofs / as->w, gridofs, f, g, h);
        //astar_print_heap (as, "*** AFTER ADDITION");
}


inline static void
astar_add_closed (astar_t * as, square_t * s, uint32_t gridofs)
{
        __debug("\t+C Adding (%d,%d) (ofs=%d) to closed list:",
                gridofs % as->w, gridofs / as->w, gridofs);
        __debug_square (as, s);

        assert (as != NULL);
        assert (&(as->grid[gridofs]) == s);
        assert (s->init);

        // Side-effect of the A* algorithm. Can't go on the closed
        // list if it wasn't on the open list before.
        assert (s->open == 1);
        
        // Statistics
        as->open--;
        as->closed++;

        // Remove square's membership in the open list and add it to the closed list.
        s->open = 0;
        s->closed = 1;
        
        // Check to see if this is the best move so far. If a solution can't be
        // found, we can use this information (best score and best square) to
        // produce a compromise.
        if (s->h < as->bestscore) {
                as->bestscore = s->h;
                as->bestofs = gridofs;
                as->have_best = 1;
        }

        //__debug ("-- Added (%d,%d) (ofs=%d, f=%u, g=%u, h=%u) to closed list.\n",
        //       gridofs % as->w, gridofs / as->w, gridofs, s->f, s->g, s->h);
        //astar_print_heap (as, "*** AFTER REMOVAL (ADDITION TO CLOSED LIST)");
}


inline static void
astar_update (astar_t * as, square_t * square, uint32_t gridofs, uint32_t g)
{
        assert (as != NULL);
        assert (square != NULL);

        // As per A* algorithm (h doesn't change because it only depends on the location
        // of the square).
        uint32_t f = g + square->h;

        // Paranoia -- we should only be updating to lower g.
        assert (&(as->grid[gridofs]) == square);
        assert (square->g > g);

        // Do we need to update the heap? If f changed, we do.
        if (f != square->f) {

                square->f = f;
                square->g = g;

                // Update the F value on the heap. Keep the heap
                // offset. Rebalance the heap.
                __debug("*** astar_heap_update: ");
                __debug_square (as, square);

                uint32_t newofs = astar_heap_update (as->heap, square);
                assert (as->heap->data[newofs] == square->f);
                as->updates++;
                __debug("++ Updated (%d,%d) (new_ofs=%d, new_f=%u, new_g=%u, h=%u).\n",
                        gridofs % as->w, gridofs / as->w, gridofs, f, g, square->h);
        } else {
                __debug("++ Updated (%d,%d) (new_ofs=%d, new_f=%u, new_g=%u, h=%u). "
                        "NO HEAP UPDATE NECESSARY.\n",
                        gridofs % as->w, gridofs / as->w, gridofs, f, g, square->h);
        }

        // Change the remaining grid values.
        //square->f = f;
        //square->g = g;
}


///////////////////////////////////////////////////////////////////////////////
//
// INITIALISATION
//
///////////////////////////////////////////////////////////////////////////////

astar_t *
astar_new (const uint32_t w,
           const uint32_t h,
           uint8_t (*get) (const uint32_t, const uint32_t),
           uint32_t  (*heuristic) (const uint32_t, const uint32_t,
                                   const uint32_t, const uint32_t))
           
{
        astar_t * as = (astar_t *) malloc (sizeof (astar_t));
        check_null (as, "astar_new(), allocating memory");

        // Force a reset.
        as->must_reset = 1;

        // Set initial configuration.
        as->w = w;
        as->h = h;

        // Initialise internal/statistics fields.
        as->max_cost = 0;
        as->timeout = 0;
        as->x0 = 0;
        as->y0 = 0;
        as->x1 = 0;
        as->y1 = 0;
        as->ofs0 = 0;
        as->ofs1 = 0;
        as->bestscore = 0xffffffff;
        as->grid_init = 0;
        as->grid_clean = 0;
        as->gets = 0;
        as->updates = 0;
        as->open = 0;
        as->closed = 0;
        as->usecs = 0;
        as->loops = 0;
        as->result = 0;
        as->str_result = NULL;
        
        // Store default configuration: costs and deltas. This allows
        // reconfiguration by the advanced user.
        as->origin_x = 0;
        as->origin_y = 0;
        as->origin_set = 0;
	as->move_8way = 1;
        memcpy (as->dx, _dx, sizeof(as->dx));
        memcpy (as->dy, _dy, sizeof(as->dy)); 
        memcpy (as->mc, _mc, sizeof(as->mc));
        as->steering_penalty = _STEER_PENALTY;
        as->heuristic_factor = _HEURISTIC_FACTOR;

        // Set the heuristic callback. Go for manhattan_distance if it hasn't been provided.
        as->heuristic = heuristic != NULL? heuristic: manhattan_distance;

        // Set the map getter callback.
        as->get = get;

        // Allocate data structures (initialise the grid to zeroes).
        uint32_t area = w * h;
        as->heap = astar_heap_new (area, area);
        as->grid = (square_t *) calloc (area, sizeof (square_t));

        __debug ("Allocated %dx%d search grid and %d-item heap, %d bytes total.\n",
                 as->w, as->h, as->heap->alloc,
                 sizeof(as) + astar_heap_sizeof(as->heap) + as->w * as->h * sizeof(square_t));

        return as;
}


///////////////////////////////////////////////////////////////////////////////
//
// CONFIGURATION FUNCTIONS
//
///////////////////////////////////////////////////////////////////////////////


void
astar_set_origin (astar_t * as, const uint32_t x, const uint32_t y)
{
        assert (as != NULL);
        as->origin_x = x;
        as->origin_y = y;
        as->origin_set = 1;
}

void
astar_set_max_cost (astar_t *as, const uint32_t max_cost)
{
        assert (as != NULL);
        as->max_cost = max_cost;
}

void
astar_set_timeout (astar_t *as, const uint32_t timeout)
{
        assert (as != NULL);
        as->timeout = timeout;
}


void
astar_set_movement_mode (astar_t * as, int mode)
{
	assert (as != NULL);
	as->move_8way = mode & 1;
}


void
astar_set_dxy (astar_t *as, const uint8_t dir, const int dx, const int dy)
{
        assert (as != NULL);
        as->dx[dir & 7] = dx;
        as->dy[dir & 7] = dy;
}


void
astar_set_cost (astar_t *as, const uint8_t dir, const uint32_t cost)
{
        assert (as != NULL);
        as->mc[dir & 7] = cost;
}


void
astar_set_steering_penalty (astar_t *as, const uint32_t steering_penalty)
{
        assert (as != NULL);
        as->steering_penalty = steering_penalty;
}


void
astar_set_heuristic_factor (astar_t *as, const uint32_t heuristic_factor)
{
        assert (as != NULL);
        as->heuristic_factor = heuristic_factor;
}


///////////////////////////////////////////////////////////////////////////////
//
// PUBLIC USE FUNCTIONS
//
///////////////////////////////////////////////////////////////////////////////


void
astar_reset (astar_t * as)
{
        assert (as != NULL);

        __debug ("Resetting A* structure for another run.\n");

        // Initialise internal/statistics fields.
        as->steps = 0;
        as->score = 0;
	set_result (as, ASTAR_NOTHING);
        as->usecs = 0;
        as->gets = 0;
        as->bestofs = 0;
        as->bestx = 0;
        as->besty = 0;
        as->have_best = 0;
        as->have_route = 0;

        // Reset the heap.
        astar_heap_clear (as->heap);

        // Reset the grid.
        astar_reset_grid (as);
}


void
astar_destroy (astar_t * as)
{
        assert (as != NULL);
        astar_heap_destroy (as->heap);
        free (as->grid);
        free (as);
}


void
astar_init_grid (astar_t * as,
                 uint32_t origin_x, uint32_t origin_y,
                 uint8_t(*get)(const uint32_t, const uint32_t))
{
        assert (as != NULL);
        assert (as->grid != NULL);
        assert (as->w > 0);
        assert (as->h > 0);

        astar_set_origin (as, origin_x, origin_y);
        as->get = get;

        register uint32_t x, y;
        register square_t * square = as->grid;
        
        for (y = 0; y < as->h; y++) {
                for (x = 0; x < as->w; x++) {
                        __get_square(as, square, x, y);
                        assert (square->init);
#ifdef ASTAR_DEBUG
                        square->ofs = mkofs (as, x, y);
#endif // ASTAR_DEBUG
                        square++;
                }
        }
        as->gets += as->w * as->h;
        as->grid_init = 1;
        as->grid_clean = 1;
}


///////////////////////////////////////////////////////////////////////////////
//
// MAIN CODE
//
///////////////////////////////////////////////////////////////////////////////


// The UTC timezone -- we don' care about timezones, we only operate on time deltas.
static struct timezone _tz = { 0, 0 };


static inline uint32_t
get_time_difference (struct timeval *t0)
{
        struct timeval t;
        gettimeofday (&t, &_tz);
        return (t.tv_sec * 1000000 + t.tv_usec) - (t0->tv_sec * 1000000 + t0->tv_usec);
}


static int
astar_mark_route (astar_t *as, uint32_t ofs)
{
        assert (as != NULL);
        assert (as->grid != NULL);
        assert (as->heap != NULL);

        /* Mark the route starting with the given offset. For each step,
         * reverse direction.
         */

        square_t * s = &as->grid[ofs];
        uint32_t dir;

	s->route = 1;
        as->steps = 0;
        while (1) {
                // Find the current square and the direction of its parent.
                dir = s->dir;

                // Find the offset of this square's parent on the route.
                uint32_t parent_ofs = ofs + as->dx[dir] + as->dy[dir] * as->w;
                __debug ("ofs=%u, dir=%u (%s), parent_ofs=%u\n", ofs, dir, names[dir], parent_ofs);

                // Set the route direction in the parent.
                ofs = parent_ofs;
                if (ofs > 0xffffff00) {
                        // We don't have a full route.
                        __debug ("*** No full route.\n");
                        return 0;
                }
                __debug ("PARENT OFS = %u\n", ofs);
                s = &as->grid [ofs];
		s->route = 1;
                
                __debug ("ROUTE STEP %d: ", as->steps);
                __debug_square (as, s);

                s->rdir = REVERSE_DIR(dir); // Opposite direction to dir.
                as->steps++;
		
		if (ofs == as->ofs0) {
			break;
		}
        }

/*
#if defined(ASTAR_DEBUG) || defined(TEST_ASTAR)
        // Print out the route if we're debugging.
        __debug ("ROUTE: ");
        ofs = as->ofs0;
        while (as->grid[ofs].route) {
                uint32_t dir = as->grid[ofs].rdir;
                __debug ("%s, ", names[dir]);
                ofs += as->dx[dir] + as->dy[dir] * as->w;
        }
        __debug ("and we're there.\n");
#endif // ASTAR_DEBUG || TEST_ASTAR
*/
        return 1;
}


static inline int
_astar_main_notfound (astar_t * as)
{
        if (!astar_heap_is_empty (as->heap)) return 0;

        // The heap is empty.
        __debug("Heap is empty.\n");
                
        // We ran out of moves to check. There's no route, but record the best
        // solution found so far.
        if (as->have_best) {
                as->bestofs = astar_find_best_compromise (as);
                as->have_route = astar_mark_route (as, as->bestofs);
                as->score = as->grid[as->bestofs].f;
                as->have_route = 1;
                __debug("Couldn't find it. Best route score %d (%d,%d).\n",
                        as->grid[as->bestofs].g, as->bestx, as->besty);
        } else {
                __debug("Couldn't find it. No compromise route find, either.\n");
        }
        
        return 1;
}


static inline int
_astar_main_blocked (astar_t * as, square_t * square,
                     uint32_t current_ofs, uint32_t x, uint32_t y)
{
        // Is this square blocked? This can only happen with the starting block,
        // really. But it could make the algorithm go into an endless
        // loop (e.g. during incremental runs when the map changes and the user isn't
        // careful enough to restart the path search. So we check every time for sanity's
        // sake.
        if (square->cost != COST_BLOCKED) return 0;

        __debug("We're embedded in a blocked square at (%d,%d)!\n", x, y);
        as->bestofs = current_ofs;
        as->bestx = x;
        as->besty = y;
        as->score = square->f;
        as->have_route = 0;
        return 1;
}


static inline int
_astar_main_found (astar_t * as, uint32_t current_ofs)
{
        if (current_ofs != as->ofs1) return 0;

        __debug("Found it! ");
        __debug_square (as, square);
        astar_mark_route (as, current_ofs);
        //astar_print (as); // This is a null statement unless debugging.
        as->bestofs = as->ofs1;
        as->bestx = as->x1;
        as->besty = as->y1;
        as->score = as->grid[as->ofs1].f;
        as->have_route = 1;
        return 1;
}


static inline int
_astar_main_timeout (astar_t * as)
{
        // Timeout set?
        if (!as->timeout) return 0;

        uint32_t d = get_time_difference (&as->t0);
        __debug("Time so far: %d microseconds.\n", d);

        // Timeout expired?
        if (d < as->timeout) return 0;
        
        // Nope, we ran out of moves to check. There's no route.
        if (as->have_best) {
                as->bestofs = astar_find_best_compromise (as);
                __debug ("BEST OFS = %u\n", as->bestofs);
                astar_mark_route (as, as->bestofs);
                as->score = as->grid[as->bestofs].f;
                __debug ("Timeout exceeded. Best route score %d (%d,%d).\n",
                         as->grid[as->bestofs].g, as->bestx, as->besty);
                as->have_route = 1;
        } else {
                __debug ("Timeout exceeded. No compromise route found.\n");
        }

        return 1;
}


static inline uint32_t
_astar_eval_g (astar_t * as, square_t * from, square_t * to, int rdir)
{
        // Directions come to us reversed: dir is in the natural
        // (start-to-destination) direction, but directions in square_t are
        // stored in A* (current-to-parent-node) direction. We choose to
        // reverse the former so we can get the right movement cost and
        // penalise direction changes.

        int dir = REVERSE_DIR (rdir);

        // Original G.
        uint32_t g = from->g;

        // Add movement cost.
        g += as->mc [dir];

        // Add cost of new square.
        g += to->cost;
        
        // Penalise direction changes. Note: 'dir' comes to us reversed (first
        // to second square). Only do this for moves other than first one (with
        // 'from' is at the starting point).
        if ((int64_t)(getofs (as, from) != (int64_t)as->ofs0) && (from->dir != dir)) {
                //__debug ("CHANGE OF DIRECTION: %s -> %s: ",
                //       names[REVERSE_DIR(from->dir)], names[dir]);
                //__debug_square (as, from);
                g += as->steering_penalty;
        }

        return g;
}


#define _astar_eval_h(as, x0, y0, x1, y1) \
        (((*as->heuristic)(x0, y0, x1, y1)) * ((as)->heuristic_factor))


static inline void
_astar_main_maybe_update_square (astar_t * as, square_t * square,
                                 square_t * adj, uint32_t adj_ofs,
                                 int dir)
{
        // This square has already been considered. Is this a
        // better path to it (lower G)?
        
        uint32_t g = _astar_eval_g (as, square, adj, dir);
        
        if (g < adj->g) {
                __debug ("\t...on the open list AND A BETTER CHOICE (new g=%u, old g=%u).\n",
                         g, adj->g);
                // This is a better route to this square. Replace
                // the routing information stored on it.
                astar_update (as, adj, adj_ofs, g);
                
                // Update the direction.
                adj->dir = REVERSE_DIR(dir);
        } else {
                __debug ("\t...already on the open list.\n");
        }
}


static inline int
astar_main_loop (astar_t * as)
{
        square_t * square = NULL;
        uint32_t   current_ofs;
        register int dir;

        // Obtain the starting square.
        current_ofs = as->ofs0;
        square = get_square (as, current_ofs, as->x0, as->y0);

        // Ensure everything is pristine.
        assert (square->g == 0);
        assert (square->h == 0);
        assert (square->open == 0);
        assert (square->closed == 0);

        ///////////////////////////////////////////////////////////////////////
        //
        // STEP 1. ADD STARTING SQUARE TO THE OPEN LIST
        //
        ///////////////////////////////////////////////////////////////////////////////
        uint32_t h = _astar_eval_h (as, as->x0, as->y0, as->x1, as->y1);
        astar_add_open (as, square, current_ofs, 0, h);


        // Now start adding squares.
        while (1) {


#ifdef ASTAR_DEBUG
                astar_print (as);
#endif // ASTAR_DEBUG


                as->loops++;
                __debug ("\n\033[0;7m *** LOOP #%u *** \033[0m\n", as->loops);
                __debug ("Current square: ");
                __debug_square (as, square);
                __debug ("\n");

                // Get the co-ordinates of the current square.
                uint32_t x = getx (as, square);
                uint32_t y = gety (as, square);
                current_ofs = getofs (as, square);

                ///////////////////////////////////////////////////////////////
                //
                // TERMINATING CONDITION: TARGET REACHED.
                //
                ///////////////////////////////////////////////////////////////

                // Have we just reached the target?
                if (_astar_main_found (as, current_ofs)) {
                        return astar_error (as, ASTAR_FOUND);
                }


                ///////////////////////////////////////////////////////////////
                //
                // TERMINATING CONDITION: TIMEOUT
                //
                ///////////////////////////////////////////////////////////////

                if (_astar_main_timeout (as)) {
                        return astar_error (as, ASTAR_TIMEOUT);
                }


                ///////////////////////////////////////////////////////////////
                //
                // TERMINATING CONDITION: HEAP EMPTY (SOLUTION NOT FOUND)
                //
                ///////////////////////////////////////////////////////////////

                if (_astar_main_notfound (as)) {
                        return astar_error (as, ASTAR_NOTFOUND);
                }


                ///////////////////////////////////////////////////////////////
                //
                // TERMINATING CONDITION: STARTING SQUARE BLOCKED
                //
                ///////////////////////////////////////////////////////////////

                if (_astar_main_blocked (as, square, current_ofs, x, y)) {
                        // ASTAR_EMBEDDED is an alias for this error condition.
                        return astar_error (as, ASTAR_AMONTILLADO);
                }


                ///////////////////////////////////////////////////////////////
                //
                // STEP 2. ADD NEIGHBOURING SQUARES
                //
                ///////////////////////////////////////////////////////////////

		// The order doesn't matter, so start at num_dirs - 1 and step
		// down to 0. This is faster (simpler loop conditionals)

                for (dir = 0; dir < NUM_DIRS; dir++) {
                        uint32_t adj_x = x + as->dx[dir];
                        uint32_t adj_y = y + as->dy[dir];

			// Odd-numbered directions are the non-cardinal
			// ones. If the movement mode is along the cardinal
			// directions, skip odd directions.
			if ((as->move_8way == 0) && (dir & 1)) continue;

                        // Ensure we're still within the bounds of the search
                        // grid. As the co-ordinates are all unsigned, reaching
                        // -1 isn't possible, but reaching MAXINT (wrap-around)
                        // is, So we only check whether the upper bound of the
                        // grid has been violated and save two comparisons that
                        // would never succeed.
                        if ((adj_x >= as->w) || (adj_y >= as->h)) continue;

                        // Rather than co-ordinates, calculate the offset of the adjacent
                        // square directly.
                        uint32_t adj_ofs = mkofs (as, adj_x, adj_y);
                        square_t * adj = get_square (as, adj_ofs, adj_x, adj_y);

                        __debug ("Step 2, dir=%d d=(%d,%d): ", dir, as->dx[dir], as->dy[dir]);
                        __debug_square (as, adj);

                        // We don't care if it's blocked.
                        if (adj->cost == COST_BLOCKED) {
                                __debug ("\t...blocked.\n");
                                continue;
                        }

                        // We don't care if it's on the closed list.
                        if (adj->closed) {
                                __debug ("\t...on the closed list.\n");
                                continue;
                        }

                        // Is it on the open list?
                        if (adj->open) {
                                _astar_main_maybe_update_square (as, square, adj, adj_ofs, dir);
                        } else {

                                // Not on the open list, add it.
                                uint32_t g = _astar_eval_g (as, square, adj, dir);
                                uint32_t h = _astar_eval_h (as,
                                                            x + as->dx[dir],
                                                            y + as->dy[dir],
                                                            as->x1,
                                                            as->y1);

                                // Only add to the open set if this move has a low enough
                                // cost.
                                if ((as->max_cost == 0) || (g < as->max_cost))
                                        astar_add_open (as, adj, adj_ofs, g, h);

                                // Set the direction of the parent square. This
                                // is the OPPOSITE direction to dir.
                                adj->dir = REVERSE_DIR(dir);
                        }
                }


                ///////////////////////////////////////////////////////////////
                //
                // STEP 3. MOVE STARTING SQUARE TO THE CLOSED LIST
                //
                ///////////////////////////////////////////////////////////////

                // Add it to the closed list.
                __debug("\nStep 3. Adding current square to closed list (ofs=%u).\n", current_ofs);
                astar_add_closed (as, square, getofs (as, square));
                assert (square->open == 0);
                assert (square->closed == 1);


                ///////////////////////////////////////////////////////////////
                //
                // STEP 4. CHOOSE THE NEXT SQUARE TO EXAMINE
                //
                ///////////////////////////////////////////////////////////////

                // Obtain the cheapest move on the open list. Pass square by
                // reference, so it can be modified. Discard squares that are
                // on the closed list as, by definition, they aren't on the
                // open list. When adding squares to the closed list, we don't
                // remove them from the heap. Instead we do this here, on
                // demand. It saves processor cycles.
                uint32_t current_f;
                __debug ("\nStep 4. Popping best next square\n");
                while (!astar_heap_is_empty (as->heap)) {
                        current_f = astar_heap_pop (as->heap, &square);
                        assert (square->f == current_f);
                        if (square->closed) {
                                __debug ("\ton Closed list: ");
                                __debug_square (as, square);
                                continue;
                        } else {
                                __debug ("\tFound: ");
                                __debug_square (as, square);
                                break;
                        }
                }

                // The heap is empty. Whoops. Loop again using the current
                // square, and the empty-heap condition will be detected and
                // acted on at that point.
                if (astar_heap_is_empty (as->heap)) continue;

                __debug ("\nStep 4. Best move: ");
                __debug_square (as, square);
                __debug ("OFFSET = %u, F (heap) = %u, F (square) = %u\n",
                         getofs (as, square), current_f, square->f);

                // Sanity check.
                assert (square->f == current_f);
                __debug("Best move: ");
                __debug_square(as, square);
        }
}


int
astar_run (astar_t *as,
           const uint32_t x0, const uint32_t y0,
           const uint32_t x1, const uint32_t y1)
{
        assert (as != NULL);
        assert (as->grid != NULL);
        assert (as->heap != NULL);

        // Store the start time.
        gettimeofday (&as->t0, &_tz);

        // Reset?
        if (as->must_reset) astar_reset (as);
        as->must_reset = 1;

        // At the end of this, the grid will initialised (perhaps partially).
        as->grid_init = 1;
        
        // Configure.
        as->x0 = x0;
        as->y0 = y0;

        as->x1 = x1;
        as->y1 = y1;

        as->ofs0 = mkofs(as, x0, y0);
        as->ofs1 = mkofs(as, x1, y1);

        // Set the default heuristic if needed.
        if (as->heuristic == NULL) {
                as->heuristic = manhattan_distance;
        }

        // Fail if the grid hasn't been initialised and as->get isn't set.
        if ((as->grid_init == 0) && (as->get == NULL)) {
                as->have_route = 0;
                return astar_error (as, ASTAR_GRID_NOT_INITIALISED);
        }

        // Fail if the origin hasn't been set.
        if (!as->origin_set) {
                as->have_route = 0;
                return astar_error (as, ASTAR_ORIGIN_NOT_SET);
        }

        __debug ("Will look for a path from (%d,%d)->(%d,%d). Estimated cost %d.\n",
                 as->x0, as->y0,
                 as->x1, as->y1,
                 (*as->heuristic) (as->x0, as->y0, as->x1, as->y1) * as->heuristic_factor);

        // Handle the trivial case here. Saves us some pain later.
        if ((as->x0 == as->x1) && (as->y0 == as->y1)) {
                as->bestofs = as->ofs1;
                as->score = 0;
                as->have_route = 0;
                return astar_error (as, ASTAR_TRIVIAL);
        }

        return astar_main_loop (as);
}


///////////////////////////////////////////////////////////////////////////////
//
// GETTING RESULTS
//
///////////////////////////////////////////////////////////////////////////////


uint32_t
astar_get_directions (astar_t *as, direction_t ** directions)
{
        assert (as != NULL);
        assert (as->grid != NULL);
        assert (as->heap != NULL);
        assert (directions != NULL);

	if (!as->have_route) {
		__debug ("astar_get_directions(): No route exists, nothing to return.\n");
		return 0;
	}

        *directions = (direction_t *) malloc (as->steps + 1 * sizeof (direction_t));
        uint8_t * dp = *directions;

        // Now form the array of directions. Start at the beginning, and
        // reverse the direction of the A* path to find the real a-to-b path.
        uint32_t ofs = as->ofs0;
	uint32_t i;
	for (i = 0; i < as->steps; i++) {
                // Obtain the direction
                uint32_t dir = as->grid[ofs].rdir;
                // Store the direction.
                *dp++ = dir;
                // Move to the next square.
                ofs += as->dx[dir] + as->dy[dir] * as->w;
        }
        // Terminate the directions (for good measure).
        *dp = DIR_END;

        // Return the number of steps in the solution.
        return as->steps;
}


void
astar_free_directions (direction_t * directions)
{
	assert (directions != NULL);
	free (directions);
}


///////////////////////////////////////////////////////////////////////////////
//
// DEBUGGING FUNCTIONS
//
///////////////////////////////////////////////////////////////////////////////


#ifdef ASTAR_DEBUG


static void
astar_print_heap (astar_t * as, const char * title)
{
        uint32_t x;
        __debug ("\n%s (length=%d)\n", title, as->heap->length);
        for (x = 0; x < as->heap->length; x++) {
                __debug("%3d. (%d,%d) [ofs %d] -> f=%d==%d %2s -> GRID "
                        "(f %3d, g %3d, h %3d, o=%d, c=%d)\n",
                        x,
                        as->heap->squares[x]->ofs % as->w,
                        as->heap->squares[x]->ofs / as->h,
                        as->heap->squares[x]->ofs,
                        as->heap->squares[x]->f,
                        as->heap->data[x],
                        names[as->heap->squares[x]->dir],
                        as->heap->squares[x]->f,
                        as->heap->squares[x]->g,
                        as->heap->squares[x]->h,
                        as->heap->squares[x]->open,
                        as->heap->squares[x]->closed
                        );
        }
        __debug("\n\n");
}


void
astar_print (astar_t * as)
{
        uint32_t y, x;
        register square_t * s = as->grid;

#ifdef TEST_ASTAR
        uint8_t grid_get (uint32_t, uint32_t);
#endif // TEST_ASTAR

        __debug("So far:\n");
        for (y = 0; y < as->h; y++) {
                for (x = 0; x < as->w; x++) {
                        if ((x == as->x0) && (y == as->y0)) {
                                __debug("\033[0;41;1m*\033[0m");
                        } else if ((x == as->x1) && (y == as->y1)) {
                                if (s->init) {
                                        __debug("\033[0;43;1m%s\033[0m", dirs[s->dir]);
                                } else {
                                        __debug("\033[0;43;1m+\033[0m", dirs[s->dir]);
                                }
                        } else if (!s->init) {
#ifdef TEST_ASTAR
                                if (grid_get (x, y) == COST_BLOCKED) {
                                        __debug("o");
                                } else {
                                        __debug(".");
                                }
#else
                                __debug("?");
#endif
                        } else if (s->route) {
                                __debug("\033[0;1m*\033[0m");
                        } else if (s->cost == 255) {
                                __debug("\033[0;44;37mo\033[0m");
                        } else if (s->open) {
                                __debug("\033[0;42;1m%s\033[0m", dirs[REVERSE_DIR(s->dir)]);
                        } else if (s->closed) {
                                __debug("\033[0;45;30m%s\033[0m", dirs[REVERSE_DIR(s->dir)]);
                        } else {
                                __debug("\033[0;1;32mo\033[0m");
                        }
                        s++;
                }
                __debug("\n");
        }
        __debug("\nStart:  (%u,%u) ofs=%u\n", as->x0, as->y0, as->ofs0);
        __debug("Target: (%u,%u) ofs=%u\n", as->x1, as->y1, as->ofs1);
        __debug("Loops:  %u\n", as->loops);
        __debug("Open:   %u places\n", as->open);
        __debug("Closed: %u places\n", as->closed);

        __debug("\nKey:\n");
#ifdef TEST_ASTAR
        __debug("  \033[0m o \033[0m Wall\n");
        __debug("  \033[0m . \033[0m Floor\n");
#else
        __debug("  \033[0m ? \033[0m Unknown\n");
#endif // TEST_ASTAR
        __debug("  \033[0;41;1m * \033[0m Origin\n");
        __debug("  \033[0;43;1m   \033[0m Target\n");
        __debug("  \033[0;1m * \033[0m Route\n");
        __debug("  \033[0;44;37m o \033[0m Blocked\n");
        __debug("  \033[0;42;1m   \033[0m Open (directions are natural start-to-destination)\n");
        __debug("  \033[0;45;30m   \033[0m Closed\n");
        __debug("  \033[0m . \033[0m Heap SENTINEL encountered\n");
        __debug("\n\n");
        //astar_print_heap(as, "Heap at beginning of next step:");
}

#else

// Remove these function calls from non-debugging code.

#define astar_print_heap(as,title)

#define astar_print(as)

#endif // ASTAR_DEBUG


///////////////////////////////////////////////////////////////////////////////
//
// TESTING
//
///////////////////////////////////////////////////////////////////////////////


#ifdef TEST_ASTAR

#define GRID_ROWS 40
#define GRID_COLS 50
#ifndef NUM_REPS
#define NUM_REPS 1
#endif // NUM_REPS


const char * grid[] = {
//                 1    1    2    2    3    3    4    4   4
//       0    5    0    5    0    5    0    5    0    5   9
        "..................................................", // 0
        "..................................................",
        "..................................................",
        "............XXXXXXXXXXXXX...XXXX..................",
        ".....O.....XXXXXXXXXXXXXXX...XXX..................",
        "..........XXX................XXX..................", // 5
        ".........XXX...XXXXXXXXXXXXXXXXX..................",
        "........XXX...XXXXXXXXXXXXXXXXXX..................",
        ".......XX.XX......................................",
        "......XX...XX........X............................",
        ".....XX.....XX......XXX.....XX....................", // 10
        "....XX.......XX....XXXXX....XX....................",
        "...XXXXXXXXXXXXX...XXXXXX...XX....................",
        "............................XX....................",
        "...XXX.......XXX...XX..XXX..XX....................",
        "...XXX......XXX...XXX..XXX..XX....................", // 15
        "...XXX...........XXXX.............................",
        "...XXX....XXX...XXXXX..XXX........................",
        "...XXX...XXX...XXXXXX..XXX..XXX...................",
        "........XXX.................XXX...................",
        ".......XXX.............X.O........................", // 20
        "......XXX..............X..........................",
        ".......XXX..................XXXXX.................",
        "........XXXXXXXXXX.....X....X...X.................",
        ".........XXXXXXXXX.....X....XXXXX.................",
        "..................................................", // 25
        "..................................................",
        "..................................................",
        "..................................................",
        "..................................................",
        "..................................................", // 30
        "..................................................",
        "..................................................",
        "..................................................",
        "..................................................",
        "..................................................", // 35
        "..................................................",
        "..................................................",
        "..................................................",
        "..................................................", // 39
        ""
};


uint8_t
grid_get (uint32_t x, uint32_t y)
{
        assert (x < GRID_COLS);
        assert (y < GRID_ROWS);
        char c = grid[y][x];

#if 0
        // Print out the grid as it's being initialised.
        static int oldy = -1;
        if (oldy != y) {
                putchar('\n');
                oldy = y;
        }
        putchar(c);
#endif

        // Return an appropriate value ('X' marks 'walls').

        if (c == 'X') return 255;
        return 1;
}


int
main (int argc, char ** argv)
{
        // Log to stdout.
#ifdef ASTAR_DEBUG
        __astar_debugfp = stdout;
#endif
#ifdef HEAP_DEBUG
        __heap_debugfp = stdout;
#endif // HEAP_DEBUG

        astar_t * as = astar_new (40, 40, grid_get, NULL);
        uint32_t result_code, i, rep;

        //astar_t * as = astar_new (0,0, 0,0, 25,20, 40);
        //astar_t * as = astar_new (0,0, 5,5, 25,20, 40);
        //astar_t * as = astar_new (0,0, 10,28, 39,39, 40);
        //astar_t * as = astar_new (0,0, 5,20, 25,20, 40);
        //astar_t * as = astar_new (0,0, 10,10, 25,20, 40); // No route possible
        //astar_t * as = astar_new (0,0, 5,20, 29,23, 40); // No route possible.
        assert (as != NULL);
        printf("Verified: astar_t structure initialised.\n");

        printf("Running A*...\n");

        // Configure and invoke A*.

        astar_set_origin (as, 0, 0);
        //astar_init_grid (as, 0,0, grid_get);    // Not really needed.
        //astar_set_max_cost (200);
        astar_set_steering_penalty (as, 20);
        //astar_set_heuristic_factor(10);
	astar_set_movement_mode (as, 4);

        // Test trivial results.
        result_code = astar_run (as, 0,0, 0,0);
        assert (result_code == ASTAR_TRIVIAL);
        // Verify trivial result.
        printf("Verified: trivial solutions are located and reported.\n");

        // Test trivially impossible routes.
        result_code = astar_run (as, 15,3, 20,3);
        assert (result_code == ASTAR_EMBEDDED);
        // Verify trivial result.
        printf("Verified: trivially impossible condition (EMBEDDED) is detected.\n");

        //astar_set_timeout (as, 1000);
	
	for (rep = 0; rep < NUM_REPS; rep++) {
		for (i = 0; i < 40; i++) {
			// Run A*.
			
			// This is only needed for debugging, so the whole map is
			// visible. Normally, the algorithm only initialises the map grids it
			// needs, when it needs them.
			//astar_init_grid (as, 0,0, grid_get);
			
			printf("Running A* (%d,%d) -> (%d,%d)\n", 1,i, 39,39-i);
			result_code = astar_run (as, 1,i, 39,39-i);
			printf("Result: %d (%s)\n", as->result, as->str_result);
			
			astar_print (as);
			assert (result_code == as->result);
			printf("Verified: result code returned and stored.\n");
			//assert (result_code == ASTAR_FOUND);
			//printf("Verified: path found.\n");
			
			printf("Result code:     %u\n", as->result);
			printf("Run for:         %u.%03u ms\n", as->usecs / 1000, as->usecs % 1000);
			printf("Steps:           %u\n", as->steps);
			printf("Score:           %u\n", as->score);
			printf("Map get()s:      %u of %u (%d%%)\n", as->gets, as->w * as->h, as->gets * 100 / (as->w * as->h));
			printf("Route available: %d\n", as->have_route);
			
			// No route, no checking!
			if (!as->have_route) continue;
			
			// Verify the directions.
			uint8_t * directions;
			uint32_t route_steps = astar_get_directions (as, &directions);
			uint32_t i, x = as->x0, y=as->y0;
			uint32_t list_steps = 0;
			
			if (route_steps) {
				printf("Verifying directions. %d step(s) returned.\n", route_steps);
				assert (route_steps == as->steps);
				printf("Verified: path steps returned and stored.\n");
				if (list_steps) {
					printf("\tStep ---: start at (%d,%d)\n", x, y);
				}
				for (i=0; directions[i] != DIR_END; i++) {
					uint8_t dir = directions[i];
					
					// End of directions?
					if (directions[i] == DIR_END) break;
					x += as->dx[dir];
					y += as->dy[dir];
					if (list_steps) {
						printf("\tStep %3d: %d (%s), now at (%d,%d)\n",
						       i + 1, directions[i], names[directions[i]],
						       x, y);
					}
					// Make sure only the lower 3 bits are used.
					assert ((directions[i] & 0xf8) == 0);
					
					// Ensure this is really part of the route.
					if ((x != as->x1) && (y != as->y1)) {
						//printf("\t\t*** ofs=%u (%d,%d), route=%u\n",
						//       mkofs(as,x,y), x,y, as->grid[mkofs(as, x, y)].route);
						assert (as->grid[mkofs(as, x, y)].route == 1);
					}
				}
				printf("Verified: path directions formatted, terminated and returned correctly.\n");
				
				// Make sure the final location given in the directions is right.
				printf("Reached (%d,%d). A* reports (%d,%d) (these should match).\n",
				       x, y, as->bestofs % as->w, as->bestofs / as->w);
				free (directions);
				
				assert (x == (as->bestofs % as->w));
				assert (y == (as->bestofs / as->w));
				printf ("Verified: bestofs co-ordinates match ending location offset.\n");
				assert (x == (as->bestx));
				assert (y == (as->besty));
				printf ("Verified: bestofs co-ordinates match ending location co-ordinates.\n");
				
				// Make sure route_steps matches the actual number of steps
				assert (i == route_steps);
				
			} else {
				printf("No route available.\n");
			}
			
			printf ("\n\n");
		}
	}

        astar_destroy (as);
        printf("All tests were successful.\n");
}

#endif // TEST_ASTAR


// End of file.
