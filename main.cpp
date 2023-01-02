#include <windows.h>
#include <iostream>

// Function that takes a decimal value between 0 and 1 and returns the corresponding
// volume level in the format required by the waveOutSetVolume function.
DWORD GetVolumeLevelFromDecimal(float volume)
{
    // Clamp the volume value to the range [0, 1].
    volume = std::max(0.0f, std::min(1.0f, volume));

    // Convert the volume value to the range [0x0000, 0xFFFF].
    return static_cast<DWORD>(volume * 0xFFFF);
}

int main()
{
    // Specify whether the sound should loop or only play once.
    bool loopSound = true;

    // Set the flags for the PlaySound function.
    DWORD flags = SND_ASYNC | SND_FILENAME;
    if (loopSound)
    {
        flags |= SND_LOOP;
    }

    // Play the wav file
    if (!PlaySound(TEXT("star.wav"), NULL, flags))
    {
        std::cerr << "Failed to play sound." << std::endl;
        return 1;
    }

    std::cout << "Playing sound..." << std::endl;

    // Gradually increase the volume an amount per time period.
    float volume = 0.0f;
    const float volumeIncrease = 0.2f;
    const float increaseDuration = 1.0f;
    while (true)
    {
        // Calculate the volume level as a function of time.
        volume += volumeIncrease;

        // Set the volume.
        waveOutSetVolume(NULL, GetVolumeLevelFromDecimal(volume));

        // Sleep for the specified time step.
        Sleep(static_cast<DWORD>(increaseDuration * 1000.0f));
    }

    return 0;
}