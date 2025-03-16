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

    static bool start_and_load_sounds(); // initialize sounds

    static void play_boil_sound(int channel, int loops);
    
    static void play_bottle_high_quality_potion_sound(int channel, int loops);
    static void play_bottle_sound(int channel, int loops); // done
    static void play_collect_item_sound(int channel, int loops); // done
    static void play_enemy_ouch_sound(int channel, int loops); // done
    static void play_drop_in_cauldron_sound(int channel, int loops); // done
    static void play_interact_menu_sound(int channel, int loops); // done
    static void play_page_flip_sound(int channel, int loops);
    static void play_stir_sound(int channel, int loops);
    static void play_throw_sound(int channel, int loops); // done
    static void play_turn_dial_sound(int channel, int loops); // done

    static void halt_boil_sound();
    static void halt_general_sound();

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

