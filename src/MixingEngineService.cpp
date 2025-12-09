#include "MixingEngineService.h"
#include <iostream>
#include <memory>
#include <cmath> // Required for std::abs

/**
 * Constructor
 */
MixingEngineService::MixingEngineService():
    decks{nullptr, nullptr},
    active_deck(1),
    auto_sync(false),
    bpm_tolerance(0)
{
    std::clog << "[MixingEngineService] Initialized with 2 empty decks." << std::endl;
}

/**
 * Destructor
 */
MixingEngineService::~MixingEngineService() {
    std::clog << "[MixingEngineService] Cleaning up decks..." << std::endl;
    for(size_t i = 0; i < 2; i++){
        if(decks[i] != nullptr){
             delete decks[i];
             decks[i] = nullptr;
        }
    }
}

/**
 * loadTrackToDeck
 */
int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {
    std::clog << "\n=== Loading Track to Deck ===" << std::endl;

    AudioTrack* trackUnwrp = track.clone().release();
    if(trackUnwrp == nullptr){
        std::cerr << "[ERROR] Track: \"" << track.get_title() << "\" failed to clone" << std::endl;
        return -1;
    }
    PointerWrapper<AudioTrack> wrpr(trackUnwrp);


    bool isFirstTrack = (decks[0] == nullptr && decks[1] == nullptr);
    int target_index;

    if (isFirstTrack) {
        target_index = 1;
    } else {
    target_index = 1 - active_deck;
    }

    std::clog << "[Deck Switch] Target deck: " << target_index << std::endl;

    if(decks[target_index] != nullptr){
        delete decks[target_index];
        decks[target_index] = nullptr;
    }

    wrpr->load();
    wrpr->analyze_beatgrid();


    if(!isFirstTrack && decks[active_deck] != nullptr && auto_sync) {
        if (!can_mix_tracks(wrpr)) {
            sync_bpm(wrpr);
        }
    }


    std::string newTitle = wrpr->get_title();
    decks[target_index] = wrpr.release();
    std::clog << "[Load Complete] '" << newTitle << "' is now loaded on deck " << target_index << std::endl;

    if(!isFirstTrack && decks[active_deck] != nullptr) {
        std::clog << "[Unload] Unloading previous deck " << active_deck
         << " (" << decks[active_deck]->get_title() << ")" << std::endl;

        delete decks[active_deck];
        decks[active_deck] = nullptr;
    }


    active_deck = target_index;

    std::clog << "[Active Deck] Switched to deck " << target_index << std::endl;

    return target_index;
}

void MixingEngineService::displayDeckStatus() const {
    std::cout << "\n=== Deck Status ===\n";
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i])
            std::cout << "Deck " << i << ": " << decks[i]->get_title() << "\n";
        else
            std::cout << "Deck " << i << ": [EMPTY]\n";
    }
    std::cout << "Active Deck: " << active_deck << "\n";
    std::cout << "===================\n";
}


bool MixingEngineService::can_mix_tracks(const PointerWrapper<AudioTrack>& track) const {
    if (decks[active_deck] == nullptr || track.get() == nullptr) {
        return false;
    }
    return std::abs(decks[active_deck]->get_bpm() - track->get_bpm()) <= bpm_tolerance;
}

void MixingEngineService::sync_bpm(const PointerWrapper<AudioTrack>& track) const {
    if (decks[active_deck] == nullptr || track.get() == nullptr) return;

    int originalBpm = track->get_bpm();
    int newBpm = (originalBpm + decks[active_deck]->get_bpm()) / 2;

    track->set_bpm(newBpm);
    std::cout << "[Sync BPM] Syncing BPM from " << originalBpm << " to " << newBpm << std::endl;
}
