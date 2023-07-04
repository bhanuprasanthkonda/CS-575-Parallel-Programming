#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

int NowYear = 2022; // 2022 - 2027
int NowMonth = 1;   // 0 - 11

float NowPrecip = 10;  // inches of rain per month
float NowTemp = 42;    // temperature this month
float NowHeight = 12.; // grain height in inches
int NowNumDeer = 10.;  // number of deer in the current population

const float GRAIN_GROWS_PER_MONTH = 9.0;
const float ONE_DEER_EATS_PER_MONTH = 1.0;

const float AVG_PRECIP_PER_MONTH = 7.0; // average
const float AMP_PRECIP_PER_MONTH = 6.0; // plus or minus
const float RANDOM_PRECIP = 2.0;        // plus or minus noise

const float AVG_TEMP = 60.0;   // average
const float AMP_TEMP = 20.0;   // plus or minus
const float RANDOM_TEMP = 3.0; // plus or minus noise

const float MIDTEMP = 40.0;
const float MIDPRECIP = 10.0;

unsigned int seed = 0;
float ang = 30;
float Growth_Rate = 1.f;
float growth_rate;

float Ranf(unsigned int *seedp, float low, float high)
{
    float r = (float)rand_r(seedp); // 0 - RAND_MAX

    return (low + r * (high - low) / (float)RAND_MAX);
}

int Ranf(unsigned int *seedp, int ilow, int ihigh)
{
    float low = (float)ilow;
    float high = (float)ihigh + 0.9999f;

    return (int)(Ranf(seedp, low, high));
}

omp_lock_t Lock;
volatile int NumInThreadTeam;
volatile int NumAtBarrier;
volatile int NumGone;

// specify how many threads will be in the barrier:
//	(also init's the Lock)

void InitBarrier(int n)
{
    NumInThreadTeam = n;
    NumAtBarrier = 0;
    omp_init_lock(&Lock);
}

// have the calling thread wait here until all the other threads catch up:

void WaitBarrier()
{
    omp_set_lock(&Lock);
    {
        NumAtBarrier++;
        if (NumAtBarrier == NumInThreadTeam)
        {
            NumGone = 0;
            NumAtBarrier = 0;
            // let all other threads get back to what they were doing
            // before this one unlocks, knowing that they might immediately
            // call WaitBarrier( ) again:
            while (NumGone != NumInThreadTeam - 1)
                ;
            omp_unset_lock(&Lock);
            return;
        }
    }
    omp_unset_lock(&Lock);

    while (NumAtBarrier != 0)
        ; // this waits for the nth thread to arrive

#pragma omp atomic
    NumGone++; // this flags how many threads have returned
}

float SQR(float x)
{
    return x * x;
}

float Deer()
{
    for (;NowMonth < 72;)
    {
        int nextNumDeer = NowNumDeer;
        int carryingCapacity = (int)(NowHeight);
        if (nextNumDeer < carryingCapacity)
            nextNumDeer++;
        else if (nextNumDeer > carryingCapacity)
            nextNumDeer--;
        if (NowMonth % 2 == 0)
            nextNumDeer += (int)(nextNumDeer * Growth_Rate) / 2;
        if (nextNumDeer < 0)
            nextNumDeer = 0;
        WaitBarrier();
        NowNumDeer = nextNumDeer;
        WaitBarrier();
        WaitBarrier();
    }
}
void Grain()
{
    for (;NowMonth < 72;)
    {
        float nextHeight = NowHeight;
        float tempFactor = exp(-SQR((NowTemp - MIDTEMP) / 10.));
        float precipFactor = exp(-SQR((NowPrecip - MIDPRECIP) / 10.));
        nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
        if (NowMonth % 2 == 1)
            nextHeight += nextHeight * Growth_Rate / 2;
        if (nextHeight < 0.)
            nextHeight = 0.;
        else if (nextHeight <= 1)
            nextHeight = 5.;
        WaitBarrier();
        NowHeight = nextHeight;
        WaitBarrier();
        WaitBarrier();
    }
}

void Watcher()
{
    for (;NowMonth < 72;)
    {
        WaitBarrier();
        WaitBarrier();
        fprintf(stderr, "%d,%f,%d,%f,%f,%f\n", NowMonth+1, NowHeight, NowNumDeer,
                NowPrecip, (5. / 9.) * (NowTemp - 32), Growth_Rate * 10);
        NowMonth++;
        ang = (30. * (float)NowMonth + 15.) * (M_PI / 180.);

        float temp = AVG_TEMP - AMP_TEMP * cos(ang);

        NowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);
        float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
        NowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);
        if (NowPrecip < 0.)
            NowPrecip = 0.;

        WaitBarrier();
    }
}
void MyAgent()
{
    for (;NowMonth < 72;)
    {

        growth_rate = Ranf(&seed, 0.f, 0.2f);
        WaitBarrier();
        Growth_Rate = growth_rate;
        WaitBarrier();
        WaitBarrier();
    }
}

int main(int argc, char *argv[])
{

    omp_set_num_threads(4); // same as # of sections
    InitBarrier(3);

#pragma omp parallel sections
    {

#pragma omp section
        {
            Deer();
        }

#pragma omp section
        {
            Grain();
        }

#pragma omp section
        {
            Watcher();
        }

#pragma omp section
        {
            MyAgent(); // your own
        }
    }

    // implied barrier -- all functions must return in order
    // to allow any of them to get past here

    return 0;
}