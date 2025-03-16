#pragma once

#include "./tinyECS/registry.hpp"
#include "./tinyECS/components.hpp"
#include "common.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_init.hpp"

#include <SDL.h>
#include <SDL_mixer.h>

class SoundSystem
{
public:

    static bool startAndLoadSounds(); // initialize sounds

    static void playBoilSound(int channel, int loops);
    static void playBottleHighQualityPotionSound(int channel, int loops);
    static void playBottleSound(int channel, int loops); // done
    static void playCollectItemSound(int channel, int loops); // done
    static void playEnemyOuchSound(int channel, int loops); // done
    static void playDropInCauldronSound(int channel, int loops); // done
    static void playInteractMenuSound(int channel, int loops); // done
    static void playPageFlipSound(int channel, int loops);
    static void playStirSound(int channel, int loops);
    static void playThrowSound(int channel, int loops); // done
    static void playTurnDialSound(int channel, int loops); // done

    static void continueBoilSound(int channel, int loops);

    static void haltBoilSound();
    static void haltGeneralSound();

    SoundSystem()
    {
    }

    ~SoundSystem(); // Destroy music components

private:
    static Mix_Chunk* boil_sound;
    static Mix_Chunk* bottle_high_quality_potion_sound;
    static Mix_Chunk* bottle_sound;
    static Mix_Chunk* collect_item_sound;
    static Mix_Chunk* drop_in_cauldron_sound;
    static Mix_Chunk* enemy_ouch_sound;
    static Mix_Chunk* interact_menu_sound;
    static Mix_Chunk* page_flip_sound;
    static Mix_Chunk* stir_sound;
    static Mix_Chunk* throw_sound;
    static Mix_Chunk* turn_dial_sound;
};

