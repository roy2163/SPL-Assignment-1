#include "DJLibraryService.h"
#include "SessionFileParser.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>
#include <filesystem>


DJLibraryService::DJLibraryService(const Playlist& playlist) 
    : playlist(playlist), library() {}
/**
 * @brief Load a playlist from track indices referencing the library
 * @param library_tracks Vector of track info from config
 */
void DJLibraryService::buildLibrary(const std::vector<SessionConfig::TrackInfo>& library_tracks) {
    for(const auto& track : library_tracks){
        if(track.type == "MP3"){
            AudioTrack* T = new MP3Track(track.title, track.artists, 
                   track.duration_seconds, track.bpm, track.extra_param1, (track.extra_param2 > 0));
            library.push_back(T);
        } else if(track.type == "WAV"){
            AudioTrack* T = new WAVTrack(track.title, track.artists, 
                   track.duration_seconds, track.bpm, track.extra_param1, track.extra_param2);
            library.push_back(T);
        }
    }
    std::cout <<  "[INFO] Track library built: " << library.size() << " tracks loaded" << std::endl;
}

/**
 * @brief Display the current state of the DJ library playlist
 * 
 */
void DJLibraryService::displayLibrary() const {
    std::cout << "=== DJ Library Playlist: " 
              << playlist.get_name() << " ===" << std::endl;

    if (playlist.is_empty()) {
        std::cout << "[INFO] Playlist is empty.\n";
        return;
    }

    // Let Playlist handle printing all track info
    playlist.display();

    std::cout << "Total duration: " << playlist.get_total_duration() << " seconds" << std::endl;
}

/**
 * @brief Get a reference to the current playlist
 * 
 * @return Playlist& 
 */
Playlist& DJLibraryService::getPlaylist() {
    // Your implementation here
    return playlist;
}

/**
 * TODO: Implement findTrack method
 * 
 * HINT: Leverage Playlist's find_track method
 */
AudioTrack* DJLibraryService::findTrack(const std::string& track_title) {
    return playlist.find_track(track_title);
}

void DJLibraryService::loadPlaylistFromIndices(const std::string& playlist_name, 
                                               const std::vector<int>& track_indices) {
    std::cout << "[INFO] Loading playlist: "  << playlist_name << std::endl;
    playlist = Playlist(playlist_name);
    for(auto it = track_indices.begin(); it != track_indices.end(); ++it){
        int i = *it;
        if(i >= 1 && i <= static_cast<int>(library.size())){
            AudioTrack* ptr = library[i - 1]->clone().release();
            if(ptr == nullptr){
                std::cerr << "[ERROR] Failed to clone track at index: " << i << std::endl;
                continue;
            }
            ptr->load();
            ptr->analyze_beatgrid();
            playlist.add_track(ptr);
            //std::cout << "Added '" << ptr->get_title() <<  "' to playlist '" <<  playlist_name << "'" << std::endl;
        }
        else{
            std::cerr << "[WARNING] Invalid track index: " << i << std::endl;
        }
    }
    std::cout << "[INFO] Playlist loaded: " << playlist_name << " (" << playlist.get_track_count() << " tracks)" << std::endl;
}
/**
 * TODO: Implement getTrackTitles method
 * @return Vector of track titles in the playlist
 */
std::vector<std::string> DJLibraryService::getTrackTitles() const {
    std::vector<std::string> titles;
    for (const auto& track : playlist.getTracks()){
        titles.push_back(track->get_title());
    }
    return titles;
}

DJLibraryService::~DJLibraryService(){
    for(auto* track : library){
        delete track;
    }
}
