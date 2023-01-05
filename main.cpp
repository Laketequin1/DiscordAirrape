#include <SFML/Audio.hpp>
#include <iostream>

int main()
{
    std::cout << "Start";

    // Load the sound buffer from a file
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("test.wav"))
        return -1;

    // Create a sound instance and play it
    sf::Sound sound;
    sound.setBuffer(buffer);
    sound.play();

    // Keep the program running until the sound finishes playing
    while (sound.getStatus() == sf::Sound::Playing)
    {
        // Do nothing
    }

    return 0;
}