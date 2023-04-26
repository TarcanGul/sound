#include <iostream>
#include <fstream>
#include "SoundApplication.hpp"
#include "Tsound.hpp"

int SoundApplication::run(int argc, char ** argv) {
    std::cout << "Welcome to sound!\n";

    if (argc > 2) {
        help();
        return 1;
    }
    std::string file = argv[1];

    TSound::playSound(file);

    return 0;
}

void SoundApplication::help() {
    std::cout << "Usage:" << std::endl;
    std::cout << "sound <soundfile>" << std::endl;
}