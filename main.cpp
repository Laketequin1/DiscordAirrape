#include <tchar.h>
#include <Windows.h>
#include <Mmsystem.h>
#include <stdio.h>
#include <iostream>
#include <future>
#include <thread>

#pragma comment(lib, "Winmm.lib")

DWORD volumeFromLevel(float level)
{
    // Clamp the level to the range [0, 1]
    level = std::max(0.0f, std::min(level, 1.0f));

    // Convert the level to a 16-bit value
    WORD volume = (WORD)(level * 65535);

    // Return the volume as a DWORD value
    return MAKELONG(volume, volume);
}

int play_sound(LPWSTR file_name, int audio_out, bool do_loop, float volume, float increment)
{
    printf("Thread started\n");

    // Open the WAV file
    HMMIO hFile = mmioOpen(file_name, NULL, MMIO_READ | MMIO_ALLOCBUF);
    if (hFile == NULL)
    {
        printf("Failed to open WAV file\n");
        return 1;
    }

    // Read the WAV file header
    MMCKINFO chunkInfo;
    chunkInfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(hFile, &chunkInfo, NULL, MMIO_FINDRIFF))
    {
        printf("Failed to read WAV file header\n");
        mmioClose(hFile, 0);
        return 1;
    }

    // Read the format chunk
    MMCKINFO formatChunkInfo;
    formatChunkInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(hFile, &formatChunkInfo, &chunkInfo, MMIO_FINDCHUNK))
    {
        printf("Failed to read WAV format chunk\n");
        mmioClose(hFile, 0);
        return 1;
    }

    // Read the format data
    WAVEFORMATEX wfx;
    if (mmioRead(hFile, (HPSTR)&wfx, formatChunkInfo.cksize) != formatChunkInfo.cksize)
    {
        printf("Failed to read WAV format data\n");
        mmioClose(hFile, 0);
        return 1;
    }

    // Ascend from the format chunk
    mmioAscend(hFile, &formatChunkInfo, 0);

    // Read the data chunk
    MMCKINFO dataChunkInfo;
    dataChunkInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hFile, &dataChunkInfo, &chunkInfo, MMIO_FINDCHUNK))
    {
        printf("Failed to read WAV data chunk\n");
        mmioClose(hFile, 0);
        return 1;
    }

    // Allocate a buffer for the WAV data
    LPSTR pBuffer = new char[dataChunkInfo.cksize];

    // Read the WAV data
    if (mmioRead(hFile, pBuffer, dataChunkInfo.cksize) != dataChunkInfo.cksize)
    {
        printf("Failed to read WAV data\n");
        delete[] pBuffer;
        mmioClose(hFile, 0);
        return 1;
    }

    // Close the WAV file
    mmioClose(hFile, 0);

    // Wait for the WAV data to finish playing
    do
    {
        // Open the correct audio output device
        HWAVEOUT hWaveOut; //    number
        if (waveOutOpen(&hWaveOut, audio_out, &wfx, 0, 0, WAVE_FORMAT_DIRECT | WAVE_ALLOWSYNC))
        {
            printf("Failed to open audio output device\n");
            delete[] pBuffer;
            return 1;
        }

        // Prepare a WAVEHDR structure for the WAV data
        WAVEHDR waveHdr;
        waveHdr.lpData = pBuffer;
        waveHdr.dwBufferLength = dataChunkInfo.cksize;
        waveHdr.dwBytesRecorded = 0;
        waveHdr.dwUser = 0;
        waveHdr.dwFlags = 0;
        waveHdr.dwLoops = 0;

        waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(waveHdr));
        waveOutSetVolume(hWaveOut, volumeFromLevel(volume));

        // Play the WAV data
        waveOutWrite(hWaveOut, &waveHdr, sizeof(waveHdr));

        while (waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
        {
            volume += increment;
            waveOutSetVolume(hWaveOut, volumeFromLevel(volume));
            if (audio_out == 0){
                std::cout << "Volume: " << volume << std::endl;
            }
            Sleep(1000);
        }

        // Clean up
        //delete[] pBuffer;
        waveOutClose(hWaveOut);

    } while (do_loop);

    printf("Thread closed\n");

    delete[] pBuffer;

    return 0;
}

int main()
{
    LPWSTR file_name = _T("audio/Outro.wav");
    bool do_loop = true;
    float volume = 0.5f;
    float increment = 0.0f;

    printf("Start main\n");

    // Launch the function asynchronously and get a future object
    std::future<int> f1 = std::async(play_sound, file_name, 0, do_loop, volume, increment);
    std::future<int> f2 = std::async(play_sound, file_name, 2, do_loop, volume, increment);
    //
    f1.wait();
    f2.wait();

    printf("Closed main\n");
    return 0;
}