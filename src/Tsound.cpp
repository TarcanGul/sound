#include "Tsound.hpp"
#include <iostream>
#include <fstream>

void TSound::playSound(std::string file) {
    //Assume file is in the directory.
    std::ifstream audiofile(file, std::ios::binary);

    //Get file size.
    audiofile.seekg(0, std::ios::end);
    size_t length = audiofile.tellg();
    audiofile.seekg(0, std::ios::beg);
    
    byte soundBytes[length];

    audiofile.read((char *) soundBytes, length);

    for(byte b : soundBytes) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned) b << ' ';
    }
    audiofile.close();
}