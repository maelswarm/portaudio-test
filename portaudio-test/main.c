//
//  main.c
//  portaudio-test
//
//  Created by fairy-slipper on 4/20/16.
//  Copyright © 2016 fairy-slipper. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "portaudio.h"

#define NUM_SECONDS   (.5f)
#define SAMPLE_RATE   (44100)
#define HERTZ   (261.6f)

double MIDI = 60.0f;

typedef struct {
    float left_phase;
    float right_phase;
    double left_x;
    double right_x;
} paSinData;

typedef struct {
    float left_phase;
    float right_phase;
    double left_x;
    double right_x;
} paSqrData;

typedef struct {
    float left_phase;
    float right_phase;
} paSawData;

int h = 0;
char *token = (char *)NULL;

/* This routine will be called by the PortAudio engine when audio is needed.
 ** It may called at interrupt level on some machines so don't do anything
 ** that could mess up the system like calling malloc() or free().
 */
static int sawCallback( const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData )
{
    /* Cast data passed through stream to our structure. */
    paSawData *data = (paSawData*)userData;
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    for( i=0; i<framesPerBuffer; i++ )
    {
        *out++ = data->left_phase;  /* left */
        *out++ = data->right_phase;  /* right */
        /* Generate note with midi conversion */
        data->left_phase += 0.000045421511627907f*pow(2.0f, ((MIDI-69.0f)/12.0f))*440.0f;
        /* When signal reaches top, drop back down. */
        if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
        /* Generate note with midi conversion */
        data->right_phase += 0.000045421511627907f*pow(2.0f, ((MIDI-69.0f)/12.0f))*440.0f;
        if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
    }
    //printf("%i\n", h++);
    return 0;
}

static int sqrCallback( const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData )
{
    /* Cast data passed through stream to our structure. */
    paSqrData *data = (paSqrData*)userData;
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    for( i=0; i<framesPerBuffer; i++ )
    {
        *out++ = data->left_phase;  /* left */
        *out++ = data->right_phase;  /* right */
        /* Generate note with midi conversion */
        data->left_x += 0.000045421511627907f*pow(2.0f, ((MIDI-69.0f)/12.0f))*440.0f;
        /* When signal reaches top, drop back down. */
        if( data->left_x >= 0.0f ) data->left_phase = 1.0f;
        if( data->left_x >= 1.0f ) { data->left_phase = -1.0f; data->left_x = -1.0f;}
        /* Generate note with midi conversion */
        data->right_x += 0.000045421511627907f*pow(2.0f, ((MIDI-69.0f)/12.0f))*440.0f;
        /* When signal reaches top, drop back down. */
        if( data->right_x >= 0.0f ) data->right_phase = 1.0f;
        if( data->right_x >= 1.0f ) { data->right_phase = -1.0f; data->right_x = -1.0f;}
    }
    //printf("%i\n", h++);
    return 0;
}

static int sinCallback( const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo* timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *userData )
{
    /* Cast data passed through stream to our structure. */
    paSinData *data = (paSinData*)userData;
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    for( i=0; i<framesPerBuffer; i++ )
    {
        *out++ = data->left_phase;  /* left */
        *out++ = data->right_phase;  /* right */
        /* Generate note with midi conversion */
        data->left_x += 0.000045421511627907f*pow(2.0f, ((MIDI-69.0f)/12.0f))*440.0f;
        data->left_phase = sin(data->left_x*M_PI);
        /* When signal reaches top, drop back down. */
        if( data->left_x >= 2.0f ) data->left_x = 0.0f;
        /* Generate note with midi conversion */
        data->right_x += 0.000045421511627907f*pow(2.0f, ((MIDI-69.0f)/12.0f))*440.0f;
        data->right_phase = sin(data->right_x*M_PI);
        /* When signal reaches top, drop back down. */
        if( data->right_x >= 2.0f ) data->right_x = 0.0f;
    }
    //printf("%i\n", h++);
    return 0;
}

/*******************************************************************/
static paSinData data;

int main(int argc, const char * argv[])
{
    if (argc!=2) {
        printf("Please specify melody file.\n");
        return 0;
    }
    
    FILE *melodyfd = fopen(argv[1], "r+");
    if (melodyfd == NULL) {
        printf("Error opening melody file!!\n");
        return 0;
    }
    
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
                               sinCallback,
                               &data );
    if( err != paNoError ) goto error;
    
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
    
    /* Sleep for several seconds. */
    
    char buf[1000];
    while (fgets(buf,1000, melodyfd)!=NULL) {
        const char t[2] = " \n";
        token = strtok(buf, t);
        
        for(int i=0; token!=NULL; i++) {
            
            MIDI = atoi(token);
            Pa_Sleep(NUM_SECONDS*1000);
            token=strtok(NULL, t);
        }
    }
    
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
