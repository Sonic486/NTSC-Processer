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
    
    

    unsigned int alignment = volk_get_alignment();
    lv_32fc_t* outputComplex = (lv_32fc_t*)volk_malloc(sizeof(lv_32fc_t)*fileinfo.frames, alignment);
    // The oscillator rotates at f=0.1
    float sinAngle = 2.0 * 3.14159265359 * 59960 / fileinfo.samplerate;
    lv_32fc_t phase_increment = lv_cmake(std::cos(sinAngle), std::sin(sinAngle));
    lv_32fc_t phase= lv_cmake(1.f, 0.0f); // start at 1 (0 rad phase)
    volk_32fc_s32fc_x2_rotator_32fc(outputComplex, inputComplex, phase_increment, &phase, fileinfo.frames);
    free(inputComplex);

    
    
    int k = 163;
    float a = 0.9f;
    for(long int i = k; i < fileinfo.frames - k; i++) {
        float real = outputComplex[i].real() + (a * outputComplex[i - k].real());
        float imag = outputComplex[i].imag() + (a * outputComplex[i - k].imag());
        std::complex<float> thecomplex(real, imag);
        outputComplex[i] = thecomplex;
    }
    


    printf("converting back to real type\n");
    float *outputReal = (float*) calloc(fileinfo.frames * fileinfo.channels, sizeof(float));
    for(long int i = 0; i < fileinfo.frames; i++) {
        outputReal[(2 * i)] = outputComplex[i].real();
        outputReal[(2 * i)+1] = outputComplex[i].imag();
    }
    free(outputComplex);

    printf("writing to file\n");
    SNDFILE *outFile;
    SF_INFO outFileInfo = fileinfo;
    outFile = sf_open("out.wav", SFM_WRITE, &outFileInfo);
    sf_writef_float(outFile, outputReal, fileinfo.frames);
	sf_close(outFile);

    
    return 0;
}
