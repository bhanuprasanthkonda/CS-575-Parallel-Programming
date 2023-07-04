#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// setting the number of threads:
#ifndef NUMT
#define NUMT 8
#endif

// #ifndef NUMTRIALS
// #define NUMTRIALS 5
// #endif

// setting the number of trials in the monte carlo simulation:
#ifndef NUMNODES
#define NUMNODES 4096
#endif

const float XMIN = -1.;
const float XMAX = 1.;
const float YMIN = -1.;
const float YMAX = 1.;

// float Height( int, int );	// function prototype

const float N = 2.5f;
const float R = 1.2f;

float Height(int iu, int iv) // iu,iv = 0 .. NUMNODES-1
{
    float x = -1. + 2. * (float)iu / (float)(NUMNODES - 1); // -1. to +1.
    float y = -1. + 2. * (float)iv / (float)(NUMNODES - 1); // -1. to +1.

    float xn = pow(fabs(x), (double)N);
    float yn = pow(fabs(y), (double)N);
    float rn = pow(fabs(R), (double)N);
    float r = rn - xn - yn;
    if (r <= 0.)
        return 0.;
    float height = pow(r, 1. / (double)N);
    return height;
}

int main(int argc, char *argv[])
{

    omp_set_num_threads(NUMT); // set the number of threads to use in parallelizing the for-loop:`
    // the area of a single full-sized tile:
    // (not all tiles are full-sized, though)
    // printf("NUMT: %d, ", NUMT);

    float fullTileArea = (((XMAX - XMIN) / (float)(NUMNODES - 1)) *
                          ((YMAX - YMIN) / (float)(NUMNODES - 1)));

    // sum up the weighted heights into the variable "volume"
    // using an OpenMP for-loop and a reduction:
    float final_volume = 0;
    float maxPerformance = 0;
    double time0 = omp_get_wtime();
    float volume = 0;

#pragma omp parallel for collapse(2) shared(fullTileArea) reduction(+ \
                                                                    : volume)
    for (int y = 0; y < NUMNODES; y++)
    {
        for (int x = 0; x < NUMNODES; x++)
        {
            float height = Height(x, y);

            if (x == 0 || x == NUMNODES - 1)
                height *= 0.5;

            if (y == 0 || y == NUMNODES - 1)
                height *= 0.5;

            volume += height;
        }
    }
    volume *= 2 * fullTileArea;
    double time1 = omp_get_wtime();
    double megaTrialsPerSecond = (double)NUMNODES * NUMNODES / (time1 - time0) / 1000000.;
    if (final_volume < volume)
        final_volume = volume;
    if (megaTrialsPerSecond > maxPerformance)
        maxPerformance = megaTrialsPerSecond;

    fprintf(stderr, "NUMT: %d, NUMNODES: %d, volume: %f, maxPerformance: %f \n", NUMT, NUMNODES, final_volume, maxPerformance);
    return 0;
}
