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

    // Set description.
    // https://developer.apple.com/documentation/coreaudiotypes/audiostreambasicdescription?language=objc

    AudioQueueOutputCallback outputCallback = atAudioQueueOutput;
    AudioStreamBasicDescription desc = {0};
    desc.mSampleRate = sampleRate;
    desc.mFormatID = kAudioFormatLinearPCM;
    desc.mBitsPerChannel = bitsPerSample / numOfChannels;
    desc.mFramesPerPacket = 1;
    desc.mChannelsPerFrame = numOfChannels;
    desc.mBytesPerFrame = 2;
    desc.mBytesPerPacket = 2;
    AudioQueueRef queue;

    // Create new audio queue
    OSStatus status = AudioQueueNewOutput(&desc, outputCallback, data, NULL, NULL, 0, &queue);

    if(status != noErr) {
        printf("New Queue Status: %d\n", (int) (status));
        return;
    }
    size_t buffer_size = 256;

    int numOfBuffers = data->length / buffer_size;

    // // The queue will consist of buffers.
    AudioQueueBufferRef buffers[numOfBuffers];
    int curPos = 0;

    std::cout << "Writing " << numOfBuffers << " buffers.";

    for(int i = 0; i < numOfBuffers; ++i) {
        OSStatus allocateStatus = AudioQueueAllocateBuffer(queue, buffer_size, &buffers[i]);
        buffers[i]->mAudioDataByteSize = buffer_size;
        // Need memcpy the buffer.
        memcpy(buffers[i]->mAudioData, &data->buffer[curPos], buffer_size);
        if(allocateStatus != noErr) {
            printf("Allocate Status: %d\n", (int) status);
            return;
        }
        curPos += buffer_size;
        OSStatus enqueueStatus = AudioQueueEnqueueBuffer(queue, buffers[i], 0, NULL);
        if(enqueueStatus != noErr) {
            printf("Enqueue Status: %d\n", (int) status);
            return;
        }
    }

    printf("playing back...\n");
    unsigned int outNumberOfFrames;

    OSStatus primeStatus = AudioQueuePrime(queue, 0, &outNumberOfFrames);

    if(primeStatus != noErr) {
        printf("Prime Status: %d\n", (int) status);
        return;
    }

    std::cout << outNumberOfFrames << " frames are decoded." << std::endl;

    OSStatus playbackStatus = AudioQueueStart(queue, 0);

    printf("done.\n");
}

void atAudioQueueOutput(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    
    // inBuffer will be the actual queue to write to. 
    // inAQ is the queue we own.
    // Data flows from the buffer to inAQ, we have to fill the buffer. 

    std::cout << "In callback\n";

    // Might have to split the audio to packets.
    //Set buffer byte size.
    SoundPlaybackData * currentData = (SoundPlaybackData *) inUserData;
    inBuffer->mAudioDataByteSize = (UInt32) currentData->length;
    
    //Writing the audio file to buffer.
    std::memcpy(inBuffer, currentData->buffer, currentData->length);
    std::cout << "Wrote to the buffer" << currentData->length << "bytes.\n";
}

void printBytes(byte bytes[], int size) {
    for(int i =0; i < size; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned) bytes[i] << ' ';
    }
}

// Assumes little endian.
unsigned int interpretBytesAsInteger(byte bytes[], int size) {
    unsigned int result = 0;

    for(int i = size-1; i >= 0; --i){
        result <<= 8;
        result |= (unsigned int) bytes[i];
    }

    return result;
}