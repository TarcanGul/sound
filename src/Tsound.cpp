#include "Tsound.hpp"

#define PRINT(s) std::cout << s << std::endl;

unsigned int interpretInt(byte bytes[], int size) {
    unsigned int result = 0;

    for(int i = size-1; i >= 0; --i){
        result <<= 8;
        result |= (unsigned int) bytes[i];
    }

    return result;
}

std::string interpretString(byte bytes[], int size) {
    std::string result;
    for(int i = 0; i < size; ++i){
        result += (char) bytes[i];
    }

    return result;
}


void TSound::playSound(std::string file) {
    //Assume file is in the directory.
    std::ifstream fileStream(file, std::ios::binary);

    byte * fileMetadata = new byte[WAV_META_LEN]; // This value will change based on other file types.

    fileStream.read((char *) fileMetadata, WAV_META_LEN);

    unsigned int numOfChannels = interpretInt(&fileMetadata[22], 2);
    unsigned int sampleRate = interpretInt(&fileMetadata[24], 4);
    unsigned int bitsPerSample = interpretInt(&fileMetadata[34], 2);
    unsigned int audioDataSize = interpretInt(&fileMetadata[40], 4);

    byte * audioData = new byte[audioDataSize];

    std::cout << "Audio data size is " << audioDataSize << std::endl;

    fileStream.read((char *) audioData, audioDataSize);

    // Read the metadata about file.
    // https://docs.fileformat.com/audio/wav/

    SoundPlaybackData * data = new SoundPlaybackData();
    data->buffer = audioData;
    data->length = audioDataSize;
    data->isPlayDone = false;
    data->isReadDone = false;
    data->buffersRead = 0;
    data->bufferSize = 0x10000;
    data->buffersCount = (data->length / data->bufferSize) + 1; // Plus one is for covering the floor of the division.

    AudioQueueOutputCallback outputCallback = atQueueEmpty;
    AudioStreamBasicDescription desc = {0};
    desc.mSampleRate = sampleRate;
    desc.mFormatID = kAudioFormatLinearPCM;
    desc.mFormatFlags = kAudioFormatFlagIsSignedInteger;
    desc.mBitsPerChannel = bitsPerSample;
    desc.mFramesPerPacket = 1; // For uncompressed audio, packets == frames.
    desc.mChannelsPerFrame = numOfChannels;
    desc.mBytesPerFrame = 4;
    desc.mBytesPerPacket = 4;

    // Create new audio queue
    OSStatus status = AudioQueueNewOutput(&desc, outputCallback, data, NULL, kCFRunLoopCommonModes, 0, &data->queue);

    if(status != noErr) {
        std::cout << "New Queue Status: " << status << std::endl;
        return;
    } 
    AudioQueueBufferRef buffers[NUM_OF_CONCRETE_BUFFERS];

    std::cout << "Writing " << data->buffersCount << " buffers.";

    for(int i = 0; i < NUM_OF_CONCRETE_BUFFERS; ++i) {
        OSStatus allocateStatus = AudioQueueAllocateBuffer(data->queue, data->bufferSize, &buffers[i]);
        if(allocateStatus != noErr) {
            std::cout << "Allocate Status:" << allocateStatus << std::endl;
            return;
        }
        outputCallback(data, data->queue, buffers[i]);
    }

    OSStatus playbackStatus = AudioQueueStart(data->queue, NULL);

    getchar();

    AudioQueueDispose(data->queue, true);

    data->isPlayDone = true;
    delete fileMetadata;
    delete[] audioData;
    delete data;
    PRINT(playbackStatus);
}

void atQueueEmpty(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    
    // inBuffer will be the actual queue to write to. 
    // inAQ is the queue we own.
    // Data flows from the buffer to inAQ, we have to fill the buffer. 

    // Might have to split the audio to packets.

    SoundPlaybackData * currentData = (SoundPlaybackData *) inUserData;

    if(currentData->isReadDone) {
        PRINT("Reading done.");
        return;
    }
    
    inBuffer->mAudioDataByteSize = inBuffer->mAudioDataBytesCapacity; 
    
    unsigned int curPos = currentData->buffersRead * currentData->bufferSize;

    memcpy(inBuffer->mAudioData, (byte *) &currentData->buffer[curPos], inBuffer->mAudioDataBytesCapacity);
    inBuffer->mAudioDataByteSize = inBuffer->mAudioDataBytesCapacity;
    
    OSStatus enqueueStatus = AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);

    if(enqueueStatus != noErr) {
        std::cout << "Enqueue Status: " << enqueueStatus << std::endl;
        return;
    }

    ++currentData->buffersRead;

    if(currentData->buffersRead >= currentData->buffersCount) {
        currentData->isReadDone = true;
        AudioQueueStop(inAQ, false);
        std::cout << "Buffers are read\n";
        return;
    }
}