/******************************************************************************!
 * \file Player.h
 * \author Sebastien Beaugrand
 * \sa http://beaugrand.chez.com/
 * \copyright CeCILL 2.1 Free Software license
 ******************************************************************************/
#pragma once
#include <nlohmann/json.hpp>
#include <mpd/client.h>

class Player
{
public:
    const unsigned int STATE_UNKNOWN = 0;
    const unsigned int STATE_PLAY = 2;
    const unsigned int STATE_PAUSE = 3;
    Player() {}
    ~Player();
    int init();
    nlohmann::json currentAlbum();
    nlohmann::json currentTitle();
    void resume(int milliseconds);
    unsigned int getPlaytime();
    nlohmann::json titleList();
    void start();
    void startId(int pos);
    void startRel(int pos);
    void pause();
    void stop();
    void m3u(std::string_view album);

    std::string musicDirectory;
private:
    int isError(const char* func);
    struct mpd_status* getMPDStatus();
    unsigned int getStatus();
    void quit();

    struct mpd_connection* mConn = nullptr;
};
