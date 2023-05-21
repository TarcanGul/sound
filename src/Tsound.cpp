#include "Tsound.hpp"

#define PRINT(s) std::cout << s << std::endl;

void printBytes(byte bytes[], int size);
unsigned int interpretBytesAsInteger(byte bytes[], int size);


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

    //Get file size.
    fileStream.seekg(0, std::ios::end);
    size_t length = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);
    
    byte * fileStreamBytes = new byte[length];

    std::cout << "Length of the file in bytes is " << length << std::endl;

    fileStream.read((char *) fileStreamBytes, length);

    // Read the metadata about file.
    // https://docs.fileformat.com/audio/wav/
    
    std::string riff = interpretString(fileStreamBytes, 4);
    int fileSize = interpretInt(&fileStreamBytes[4], 4);
    std::string wave = interpretString(&fileStreamBytes[8], 4);
    std::string fmt = interpretString(&fileStreamBytes[12], 4);
    unsigned int sizeOfChunk = interpretInt(&fileStreamBytes[16], 4);
    unsigned int fileFormat = interpretInt(&fileStreamBytes[20], 2);
    unsigned int numOfChannels = interpretInt(&fileStreamBytes[22], 2);
    unsigned int sampleRate = interpretInt(&fileStreamBytes[24], 4);
    unsigned int dataRate = interpretInt(&fileStreamBytes[28], 4);
    unsigned int blockAlignment = interpretInt(&fileStreamBytes[32], 2);
    unsigned int bitsPerSample = interpretInt(&fileStreamBytes[34], 2);
    std::string dataHeader = interpretString(&fileStreamBytes[36], 4);
    unsigned int audioDataSize = interpretInt(&fileStreamBytes[40], 4); 

    PRINT(riff);
    PRINT(fileSize);
    PRINT(wave);
    PRINT(fmt);
    PRINT(sizeOfChunk);
    PRINT(fileFormat);
    PRINT(numOfChannels);
    PRINT(sampleRate);
    PRINT(dataRate);
    PRINT(bitsPerSample);
    PRINT(blockAlignment);
    PRINT(dataHeader);
    PRINT(audioDataSize);

    SoundPlaybackData * data = new SoundPlaybackData();
    data->buffer = &fileStreamBytes[44]; //start of audio data in wav
    data->length = audioDataSize;
    data->isPlayDone = false;
    data->isReadDone = false;
    data->buffersRead = 0;
    data->bufferSize = 0x10000;
    data->buffersCount = data->length / data->bufferSize;

    // Set description.
    // https://developer.apple.com/documentation/coreaudiotypes/audiostreambasicdescription?language=objc

    AudioQueueOutputCallback outputCallback = atAudioQueueOutput;
    AudioStreamBasicDescription desc = {0};
    desc.mSampleRate = sampleRate;
    desc.mFormatID = kAudioFormatLinearPCM;
    desc.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    desc.mBitsPerChannel = bitsPerSample;
    desc.mFramesPerPacket = 1; // For uncompressed audio, packets == frames.
    desc.mChannelsPerFrame = numOfChannels;
    desc.mBytesPerFrame = 4;
    desc.mBytesPerPacket = 4;

    // Create new audio queue
    OSStatus status = AudioQueueNewOutput(&desc, outputCallback, data, NULL, kCFRunLoopCommonModes, 0, &data->queue);

    if(status != noErr) {
        printf("New Queue Status: %d\n", (int) (status));
        return;
    } 

    unsigned int initNumBuffers = 3;
    // // The queue will consist of buffers.
    AudioQueueBufferRef buffers[initNumBuffers];

    std::cout << "Writing " << data->buffersCount << " buffers.";

    for(int i = 0; i < initNumBuffers; ++i) {
        OSStatus allocateStatus = AudioQueueAllocateBuffer(data->queue, data->bufferSize, &buffers[i]);
        if(allocateStatus != noErr) {
            printf("Allocate Status: %d\n", (int) allocateStatus);
            return;
        }
        outputCallback(data, data->queue, buffers[i]);
    }

    OSStatus playbackStatus = AudioQueueStart(data->queue, NULL);

    getchar();

    AudioQueueDispose(data->queue, true);

    data->isPlayDone = true;
    delete[] fileStreamBytes;
    delete data;
    PRINT(playbackStatus);
}

void atAudioQueueOutput(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    
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
        printf("Enqueue Status: %d\n", (int) enqueueStatus);
        return;
    }

    // unsigned int outBufferPrepared;
    // OSStatus primeStatus = AudioQueuePrime(inAQ, 0, &outBufferPrepared);
    // std::cout << "Numbers of frame prepared:";
    // PRINT(outBufferPrepared);
    PRINT(currentData->buffersRead);
    ++currentData->buffersRead;

    if(currentData->buffersRead >= currentData->buffersCount) {
        currentData->isReadDone = true;
        AudioQueueStop(inAQ, false);
        std::cout << "Buffers are read\n";
        return;
    }
}

void printBytes(byte bytes[], int size) {
    for(int i =0; i < size; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned) bytes[i] << ' ';
    }
}