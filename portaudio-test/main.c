//
//  main.c
//  portaudio-test
//
//  Created by fairy-slipper on 4/20/16.
//  Copyright © 2016 fairy-slipper. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include "portaudio.h"
#define NUM_SECONDS   (.5f)
#define SAMPLE_RATE   (44100)
#define HERTZ   (261.6.0f)

double MIDI = 60.0f;

typedef struct
{
    float left_phase;
    float right_phase;
}
paTestData;

int h = 0;

/* This routine will be called by the PortAudio engine when audio is needed.
 ** It may called at interrupt level on some machines so don't do anything
 ** that could mess up the system like calling malloc() or free().
 */
static int patestCallback( const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData )
{
    /* Cast data passed through stream to our structure. */
    paTestData *data = (paTestData*)userData;
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    for( i=0; i<framesPerBuffer; i++ )
    {
        *out++ = data->left_phase;  /* left */
        *out++ = data->right_phase;  /* right */
        /* Generate middle c note with midi conversion */
        data->left_phase += 0.000045421511627907f*pow(2.0f, ((MIDI-69.0f)/12.0f))*440.0f;
        /* When signal reaches top, drop back down. */
        if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
        /* Generate middle c note with midi conversion */
        data->right_phase += 0.000045421511627907f*pow(2.0f, ((MIDI-69.0f)/12.0f))*440.0f;
        if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
    }
    //printf("%i\n", h++);
    return 0;
}

/*******************************************************************/
static paTestData data;
int main(void);
int main(void)
{
    PaStream *stream;
    PaError err;
    
    printf("PortAudio Test: output sawtooth wave.\n");
    /* Initialize our data for use by callback. */
    data.left_phase = data.right_phase = 0.0;
    /* Initialize library before making any other calls. */
    err = Pa_Initialize();
    if( err != paNoError ) goto error;
    
    /* Open an audio I/O stream. */
    err = Pa_OpenDefaultStream( &stream,
                               0,          /* no input channels */
                               2,          /* stereo output */
                               paFloat32,  /* 32 bit floating point output */
                               SAMPLE_RATE,
                               256,        /* frames per buffer */
                               patestCallback,
                               &data );
    if( err != paNoError ) goto error;
    
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
    
    /* Sleep for several seconds. */
    Pa_Sleep(NUM_SECONDS*1000);
    MIDI = 62.0f;
    Pa_Sleep(NUM_SECONDS*1000);
    MIDI = 64.0f;
    Pa_Sleep(NUM_SECONDS*1000);
    MIDI = 65.0f;
    Pa_Sleep(NUM_SECONDS*1000);
    MIDI = 67.0f;
    Pa_Sleep(NUM_SECONDS*1000);
    MIDI = 69.0f;
    Pa_Sleep(NUM_SECONDS*1000);
    MIDI = 71.0f;
    Pa_Sleep(NUM_SECONDS*1000);
    MIDI = 72.0f;
    Pa_Sleep(NUM_SECONDS*1000);
    
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;
    Pa_Terminate();
    printf("Test finished.\n");
    return err;
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}
