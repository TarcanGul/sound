#include <iostream>
#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioToolbox.h>
#include <fstream>

#define NUM_OF_CONCRETE_BUFFERS 3

typedef unsigned char byte;

struct SoundPlaybackData {
    byte * buffer;
    size_t length; // length in bytes
    AudioQueueRef queue;
    bool isPlayDone;
    bool isReadDone;
    int buffersCount;
    int bufferSize;
    size_t buffersRead;
};

// Callbacks
void atAudioQueueOutput(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);

class TSound {
    public:
        static void playSound(std::string file);
};