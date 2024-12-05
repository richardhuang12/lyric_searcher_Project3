#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <mmeapi.h>
#include <mmsystem.h>
#include <Windows.h>
#include <mutex>
#include "filedeal.h"

// Used to monitor if audio is currently playing
std::atomic<bool> isPlaying(false);

// Used to monitor if audio is stopping
std::atomic<bool> isStopping(false);

void stopAudio();

// Structure to represent song lyrics with a timestamp
struct Lyric {
    int timestamp; // Timestamp of the lyric in milliseconds
    std::string text; // The lyric text
};

#pragma comment(lib, "Winmm.lib") // Link with the Windows multimedia library

// Function to open an audio file
bool open(const std::wstring& filePath) {
    // Convert the wide string file path to a UTF-8 encoded string
    std::string utf8FilePath = wstring2string(filePath);

    // Command to open the audio file
    char command[512];
    snprintf(command, sizeof(command), "open \"%s\" type mpegvideo alias mp3", utf8FilePath.c_str());

    // Use mciSendStringA to send the command and check for errors
    if (mciSendStringA(command, NULL, 0, NULL) != 0) {
        std::cerr << "Error opening file!" << std::endl;
        return false;
    }
    return true;
}

// Function to play the audio
void playAudio() {
    const char* playCommand = "play mp3 notify";
    // Send the command to play the audio
    if (mciSendStringA(playCommand, NULL, 0, NULL) != 0) {
        std::cerr << "Error playing file!" << std::endl;
        return;
    }
}

// Function to stop the audio
void stopAudio() {
    // Stop the audio
    mciSendStringA("stop mp3", NULL, 0, NULL);
}

// Function to close the audio file
void closeAudio() {
    // Stop and close the audio
    mciSendStringA("close mp3", NULL, 0, NULL);
}
