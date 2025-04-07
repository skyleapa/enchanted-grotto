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
    static void playBottleSound(int channel, int loops);
    static void playCollectItemSound(int channel, int loops);
    static void playDialChangeSound(int channel, int loops);
    static void playEnemyOuchSound(int channel, int loops);
    static void playDropInBowlSound(int channel, int loops);
    static void playDropInCauldronSound(int channel, int loops);
    static void playGrindSound(int channel, int loops);
    static void playGulpSound(int channel, int loops);
    static void playInteractMenuSound(int channel, int loops);
    static void playPageFlipSound(int channel, int loops);
    static void playPlayerOuchSound(int channel, int loops);
    static void playStirSound(int channel, int loops);
    static void playThrowSound(int channel, int loops);
    static void playTurnDialSound(int channel, int loops);
    static void playWalkSound(int channel, int loops);

    static void continueBoilSound(int channel, int loops);

    static void haltBoilSound();
    static void haltGeneralSound();

    SoundSystem()
    {
    }

    ~SoundSystem(); // Destroy music components

private:
    static Mix_Music* background_music;
    static Mix_Chunk* boil_sound;
    static Mix_Chunk* bottle_high_quality_potion_sound;
    static Mix_Chunk* bottle_sound;
    static Mix_Chunk* collect_item_sound;
    static Mix_Chunk* dial_change_sound;
    static Mix_Chunk* drop_in_bowl_sound;
    static Mix_Chunk* drop_in_cauldron_sound;
    static Mix_Chunk* enemy_ouch_sound;
    static Mix_Chunk* grind_sound;
    static Mix_Chunk* gulp_sound;
    static Mix_Chunk* interact_menu_sound;
    static Mix_Chunk* page_flip_sound;
    static Mix_Chunk* player_ouch_sound;
    static Mix_Chunk* stir_sound;
    static Mix_Chunk* throw_sound;
    static Mix_Chunk* turn_dial_sound;
    static Mix_Chunk* walk_sound;
};

