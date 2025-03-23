#include "sound_system.hpp"

// Define static variables
Mix_Chunk* SoundSystem::boil_sound = nullptr;
Mix_Chunk* SoundSystem::bottle_high_quality_potion_sound = nullptr;
Mix_Chunk* SoundSystem::bottle_sound = nullptr;
Mix_Chunk* SoundSystem::collect_item_sound = nullptr;
Mix_Chunk* SoundSystem::dial_change_sound = nullptr;
Mix_Chunk* SoundSystem::drop_in_bowl_sound = nullptr;
Mix_Chunk* SoundSystem::drop_in_cauldron_sound = nullptr;
Mix_Chunk* SoundSystem::enemy_ouch_sound = nullptr;
Mix_Chunk* SoundSystem::grind_sound = nullptr;
Mix_Chunk* SoundSystem::gulp_sound = nullptr;
Mix_Chunk* SoundSystem::interact_menu_sound = nullptr;
Mix_Chunk* SoundSystem::page_flip_sound = nullptr;
Mix_Chunk* SoundSystem::stir_sound = nullptr;
Mix_Chunk* SoundSystem::throw_sound = nullptr;
Mix_Chunk* SoundSystem::turn_dial_sound = nullptr;

// initialize sounds
bool SoundSystem::startAndLoadSounds()
{

    //////////////////////////////////////
    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        fprintf(stderr, "Failed to open audio device");
        return false;
    }

    boil_sound = Mix_LoadWAV(audio_path("boil.wav").c_str());
    bottle_high_quality_potion_sound = Mix_LoadWAV(audio_path("bottle_high_quality_potion.wav").c_str());
    bottle_sound = Mix_LoadWAV(audio_path("bottle.wav").c_str());
    collect_item_sound = Mix_LoadWAV(audio_path("collect_item.wav").c_str());
    dial_change_sound = Mix_LoadWAV(audio_path("dial_change.wav").c_str());
    drop_in_bowl_sound = Mix_LoadWAV(audio_path("drop_in_bowl.wav").c_str());
    drop_in_cauldron_sound = Mix_LoadWAV(audio_path("drop_in_cauldron.wav").c_str());
    enemy_ouch_sound = Mix_LoadWAV(audio_path("enemy_ouch.wav").c_str());
    grind_sound = Mix_LoadWAV(audio_path("grind.wav").c_str());
    gulp_sound = Mix_LoadWAV(audio_path("gulp.wav").c_str());
    interact_menu_sound = Mix_LoadWAV(audio_path("interact_menu.wav").c_str());
    page_flip_sound = Mix_LoadWAV(audio_path("page_flip.wav").c_str());
    stir_sound = Mix_LoadWAV(audio_path("stir.wav").c_str());
    throw_sound = Mix_LoadWAV(audio_path("throw.wav").c_str());
    turn_dial_sound = Mix_LoadWAV(audio_path("turn_dial.wav").c_str());

    // return false if any sound failed to load
    return !(!boil_sound || !bottle_high_quality_potion_sound || !bottle_sound || !collect_item_sound || !dial_change_sound || !drop_in_bowl_sound || !drop_in_cauldron_sound
        || !enemy_ouch_sound || !grind_sound || !gulp_sound || !interact_menu_sound || !page_flip_sound || !stir_sound || !throw_sound || !turn_dial_sound);
}

SoundSystem::~SoundSystem()
{
    if (boil_sound) Mix_FreeChunk(boil_sound);
    if (bottle_high_quality_potion_sound) Mix_FreeChunk(bottle_high_quality_potion_sound);
    if (bottle_sound) Mix_FreeChunk(bottle_sound);
    if (collect_item_sound) Mix_FreeChunk(collect_item_sound);
    if (dial_change_sound) Mix_FreeChunk(dial_change_sound);
    if (drop_in_bowl_sound) Mix_FreeChunk(drop_in_bowl_sound);
    if (drop_in_cauldron_sound) Mix_FreeChunk(drop_in_cauldron_sound);
    if (enemy_ouch_sound) Mix_FreeChunk(enemy_ouch_sound);
    if (grind_sound) Mix_FreeChunk(grind_sound);
    if (gulp_sound) Mix_FreeChunk(gulp_sound);
    if (interact_menu_sound) Mix_FreeChunk(interact_menu_sound);
    if (page_flip_sound) Mix_FreeChunk(page_flip_sound);
    if (stir_sound) Mix_FreeChunk(stir_sound);
    if (turn_dial_sound) Mix_FreeChunk(turn_dial_sound);

    Mix_CloseAudio();
}

void SoundSystem::playBoilSound(int channel, int loops) {
    Mix_FadeInChannel(channel, boil_sound, loops, 2000); // fade this in so you can hear the dial click
}

void SoundSystem::playBottleHighQualityPotionSound(int channel, int loops) {
    Mix_PlayChannel(channel, bottle_high_quality_potion_sound, loops);
}

void SoundSystem::playBottleSound(int channel, int loops) {
    Mix_PlayChannel(channel, bottle_sound, loops);
}

void SoundSystem::playCollectItemSound(int channel, int loops) {
    Mix_PlayChannel(channel, collect_item_sound, loops);
}

void SoundSystem::playDialChangeSound(int channel, int loops) {
    Mix_PlayChannel(channel, dial_change_sound, loops);
}

void SoundSystem::playDropInBowlSound(int channel, int loops) {
    Mix_PlayChannel(channel, drop_in_bowl_sound, loops);
}

void SoundSystem::playDropInCauldronSound(int channel, int loops) {
    Mix_PlayChannel(channel, drop_in_cauldron_sound, loops);
}

void SoundSystem::playEnemyOuchSound(int channel, int loops) {
    Mix_PlayChannel(channel, enemy_ouch_sound, loops);
}

void SoundSystem::playGrindSound(int channel, int loops) {
    Mix_PlayChannel(channel, grind_sound, loops);
}

void SoundSystem::playGulpSound(int channel, int loops) {
    Mix_PlayChannel(channel, gulp_sound, loops);
}

void SoundSystem::playInteractMenuSound(int channel, int loops) {
    Mix_PlayChannel(channel, interact_menu_sound, loops);
}

void SoundSystem::playPageFlipSound(int channel, int loops) {
    Mix_PlayChannel(channel, page_flip_sound, loops);
}

void SoundSystem::playStirSound(int channel, int loops) {
    Mix_PlayChannel(channel, stir_sound, loops);
}

void SoundSystem::playThrowSound(int channel, int loops) {
    Mix_PlayChannel(channel, throw_sound, loops);
}

void SoundSystem::playTurnDialSound(int channel, int loops) {
    Mix_PlayChannel(channel, turn_dial_sound, loops);
}

void SoundSystem::continueBoilSound(int channel, int loops) {
    Mix_PlayChannel(channel, boil_sound, loops);
}
void SoundSystem::haltBoilSound() {
    Mix_FadeOutChannel((int)SOUND_CHANNEL::BOILING, 500);
}

void SoundSystem::haltGeneralSound() {
    Mix_HaltChannel((int)SOUND_CHANNEL::GENERAL);
}