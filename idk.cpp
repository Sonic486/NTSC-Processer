#include <iostream>
#include <complex>
#include <sndfile.h>
#include <volk/volk.h>


int main() {
    //Read the WAV file and get some info on it, print that info
    SNDFILE *infile;
    SF_INFO fileinfo;
    infile = sf_open("test.wav", SFM_READ, &fileinfo);
    if (infile == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    if (fileinfo.channels != 2) {
        printf("Error: file is not IQ.\n");
        return 1;
    }
    printf("File info:\nSamplerate: %d\nFrames/Samples: %lu\nChannels: %d\nFormat: %d\nSections: %d\nSeekable: %d\n", fileinfo.samplerate, fileinfo.frames, fileinfo.channels, fileinfo.format, fileinfo.sections, fileinfo.seekable);
    
    //Read the file into an array + convert to complex foat - This is some crappy code, could definitely be better
    float *InFileArray = (float*) calloc(fileinfo.frames * fileinfo.channels, sizeof(float));
    sf_readf_float(infile, InFileArray, fileinfo.frames); //Should probably not do this, later read the file in shorter pieces since that way it will not use an infinite amount of memory
    std::complex<float> *inputComplex = (std::complex<float>*) calloc(fileinfo.frames, sizeof(std::complex<float>));
    for(long int i = 0; i < fileinfo.frames; i++) {
        std::complex<float> thecomplex(InFileArray[(2 * i)], InFileArray[(2 * i)+1]); //i don't know if the first channel is real or imaginary, probably is real though
        inputComplex[i] = thecomplex;
        //Also there probably is a much be a better way to assign a complex number to an array?
    }
    free(InFileArray);
    

    

    
    return 0;
}
