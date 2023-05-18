#include <iostream>
// #include <Foundation/Foundation.h>
#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioToolbox.h>
#include <fstream>

typedef unsigned char byte;

struct SoundPlaybackData {
    byte * buffer;
    size_t length; // length in bytes
};

// Callbacks
void atAudioQueueOutput(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);

class TSound {
    public:
        static void playSound(std::string file);
};