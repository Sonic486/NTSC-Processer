#include <iostream>
#include <sndfile.h>

int main() {
    //Read the WAV file and get some info on it, print that info
    SNDFILE *infile;
    SF_INFO fileinfo;
    infile = sf_open("test.wav", SFM_READ, &fileinfo);
    if (infile == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    printf("File info:\nSamplerate: %d\nFrames/Samples: %lu\nChannels: %d\nFormat: %d\nSections: %d\nSeekable: %d\n", fileinfo.samplerate, fileinfo.frames, fileinfo.channels, fileinfo.format, fileinfo.sections, fileinfo.seekable);


    return 0;
}
