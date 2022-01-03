#include <iostream>
#include <complex>
#include <sndfile.h>
#include <volk/volk.h>
#include <string>
#include <algorithm>


#define M_PI 3.14159265358979323846 //Love you, Microsoft


bool doesFileExist(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}


bool YesNoQuestion(const std::string& question)
{
    std::string input;
    while (true)
    {
        std::cout << question << "? [y/n] ";
        std::cin >> input;
        std::transform(input.begin(), input.end(), input.begin(), ::toupper);
        if (input.find("Y") != std::string::npos || input.find("N") != std::string::npos)
        {
            if (input.find("Y") != std::string::npos)
            {
                return true;
            }
            return false;
        }
        std::cout << "Invalid Input!\n";
    }
}


int main() {
    std::string InputFileName;
    std::string OutputFileName;
    std::cout << "Input file path/name? ";
    std::getline(std::cin, InputFileName);
    if (!doesFileExist(InputFileName))
    {
        std::cout << "File " << InputFileName << " does not exist!";
        return EXIT_FAILURE;
    }
    std::cout << "Output file path/name? ";
    std::getline(std::cin, OutputFileName);
    if (doesFileExist(OutputFileName))
    {
        if (!YesNoQuestion("The file you would like to output to(" + OutputFileName + ") exists already! Would you like to overwrite it"))
        {
            return EXIT_FAILURE;
        }
    }
    

    //Read the WAV file and get some info on it, print that info
    SNDFILE* infile;
    SF_INFO fileinfo;
    infile = sf_open(InputFileName.c_str(), SFM_READ, &fileinfo);
    if (infile == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    if (fileinfo.channels != 2) {
        printf("Error: file is not IQ.\n");
        return 1;
    }

    printf("File info:\nSamplerate: %d\nFrames/Samples: %lu\nChannels: %d\nFormat: %d\nSections: %d\nSeekable: %d\n", fileinfo.samplerate, fileinfo.frames, fileinfo.channels, fileinfo.format, fileinfo.sections, fileinfo.seekable);
    
    unsigned int volk_alignment = volk_get_alignment();
    float* InFileArray = (float*)volk_malloc(sizeof(float) * fileinfo.frames * fileinfo.channels, volk_alignment);
    sf_readf_float(infile, InFileArray, fileinfo.frames); //Should probably not do this, later read the file in shorter pieces since that way it will not use an infinite amount of memory <-- "later"? it'll stay this way forever, i'm too lazy
    std::complex<float>* inputComplex = reinterpret_cast<std::complex<float>*>(InFileArray);

    //Remove DC offset, or at least try to
    if (YesNoQuestion("Remove DC spike"))
    {
        printf("Removing DC offset\n");
        int DCOffsetAverage = 10000; //Smaller value = bigger notch near DC
        for (long int i = 0; i < fileinfo.frames - DCOffsetAverage; i += DCOffsetAverage) { //Need to fix later
            float averageReal = 0;
            float averageImag = 0;
            for (int j = 0; j < DCOffsetAverage; j++) {
                averageReal += inputComplex[i + j].real();
            }
            for (int j = 0; j < DCOffsetAverage; j++) {
                averageImag += inputComplex[i + j].imag();
            }
            averageReal /= DCOffsetAverage;
            averageImag /= DCOffsetAverage;
            for (int j = 0; j < DCOffsetAverage; j++) {
                inputComplex[i + j] -= std::complex<float>(averageReal, averageImag);
            }
        }
    }

    

    if (YesNoQuestion("Wanna mix"))
    {
        int carrier;
        std::cout << "Mix Frequency(Hz)? ";
        std::cin >> carrier;
        //outputComplex = (lv_32fc_t*)volk_malloc(sizeof(lv_32fc_t) * fileinfo.frames, volk_alignment);
        float sinAngle = 2.0 * M_PI * carrier / fileinfo.samplerate;
        lv_32fc_t phase_increment = lv_cmake(std::sin(sinAngle), std::cos(sinAngle));
        if (YesNoQuestion("Is your mix frequency negative"))
        {
            phase_increment = lv_cmake(std::cos(sinAngle), std::sin(sinAngle));
        }
        lv_32fc_t phase = lv_cmake(1.f, 0.0f);
        printf("Mixing\n");
        volk_32fc_s32fc_x2_rotator_32fc(inputComplex, inputComplex, phase_increment, &phase, fileinfo.frames);
    }
    
    if (YesNoQuestion("Comb filter"))
    {
        printf("Applying comb filter\n");
        int combFrequency = 15625;
        if (YesNoQuestion("Custom comb filter frequency"))
        {
            std::cout << "Comb filter frequency: ";
            std::cin >> combFrequency;
        }
        int k = std::round((float)fileinfo.samplerate / combFrequency);
        float a = 0.4f;
        if (YesNoQuestion("Custom comb filter feedback amplitude"))
        {
            std::cout << "Comb filter a (0-1): ";
            std::cin >> a;
        }
        for (long int i = k; i < fileinfo.frames - k; i++) {
            float real = inputComplex[i].real() + (a * inputComplex[i - k].real());
            float imag = inputComplex[i].imag() + (a * inputComplex[i - k].imag());
            inputComplex[i] = std::complex<float>(real, imag);
        }
    }

    float* outputReal = reinterpret_cast<float*>(inputComplex);

    printf("writing to output file\n");
    SNDFILE* outFile;
    SF_INFO outFileInfo = fileinfo;
    outFile = sf_open(OutputFileName.c_str(), SFM_WRITE, &outFileInfo);
    sf_writef_float(outFile, outputReal, fileinfo.frames);
    sf_close(outFile);

    volk_free(inputComplex);

    return EXIT_SUCCESS;
}
