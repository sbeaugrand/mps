/******************************************************************************!
 * \file Server.cpp
 * \author Sebastien Beaugrand
 * \sa http://beaugrand.chez.com/
 * \copyright CeCILL 2.1 Free Software license
 ******************************************************************************/
#include <chrono>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include "Server.h"
#include "List.h"
#include "Player.h"
#include "path.h"
#include "log.h"

/******************************************************************************!
 * \fn Server
 ******************************************************************************/
Server::Server(List& list,
               Player& player)
    : mList(list)
    , mPlayer(player)
{
    this->Add("list", GetHandle(&Server::list, *this), {});
    this->Add("info", GetHandle(&Server::info, *this), {});
    this->Add("rand", GetHandle(&Server::rand, *this), {});
    this->Add("ok", GetHandle(&Server::ok, *this), {});
    this->Add("play", GetHandle(&Server::play, *this), {});
    this->Add("pause", GetHandle(&Server::pause, *this), {});
    this->Add("stop", GetHandle(&Server::stop, *this), {});
    this->Add("prev", GetHandle(&Server::prev, *this), {});
    this->Add("next", GetHandle(&Server::next, *this), {});
    this->Add("artist", GetHandle(&Server::artist, *this), {});
    this->Add("album", GetHandle(&Server::album, *this), { "artist", "pos" });
    this->Add("pos", GetHandle(&Server::pos, *this), { "pos" });
    this->Add("dir", GetHandle(&Server::dir, *this), { "path" });
    this->Add("musicDirectory", GetHandle(&Server::musicDirectory, *this), {});
    this->Add("checksum", GetHandle(&Server::checksum, *this), {});
    this->Add("quit", GetHandle(&Server::quit, *this), {});
}

/******************************************************************************!
 * \fn ~Server
 ******************************************************************************/
Server::~Server()
{
    DEBUG("");
}

/******************************************************************************!
 * \fn list
 ******************************************************************************/
nlohmann::json
Server::list()
{
    DEBUG("");
    nlohmann::json result = mPlayer.titleList();
    return result;
}

/******************************************************************************!
 * \fn info
 ******************************************************************************/
nlohmann::json
Server::info()
{
    DEBUG("");
    nlohmann::json result = mPlayer.currentTitle();
    result["abrev"] = mAbrev;
    return result;
}

/******************************************************************************!
 * \fn rand
 ******************************************************************************/
nlohmann::json
Server::rand()
{
    DEBUG("");
    const auto [path, abrev, days] = mList.rand();
    mSelect = path;
    mSelectTime = std::chrono::steady_clock::now();
    mAbrev = abrev;
    const auto [arti, date, albu] = ::splitPath(path);
    nlohmann::json result;
    result["artist"] = arti;
    result["date"] = date;
    result["album"] = albu;
    result["abrev"] = abrev;
    result["days"] = days;
    return result;
}

/******************************************************************************!
 * \fn ok
 ******************************************************************************/
nlohmann::json
Server::ok()
{
    DEBUG("");
    if (! mSelect.empty() && mSelectTime !=
        std::chrono::time_point<std::chrono::steady_clock>::min()) {
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<double> diff = now - mSelectTime;
        if (diff.count() < 20) {
            mPlayer.m3u(mSelect);
            mList.writeLog(mSelect);
            mSelectTime =
                std::chrono::time_point<std::chrono::steady_clock>::min();
            nlohmann::json js = mPlayer.currentTitle();
            js["cs"] = -1;
            return js;
        } else {
            mAbrev.clear();
            mSelectTime =
                std::chrono::time_point<std::chrono::steady_clock>::min();
        }
    }
    return this->pause();
}

/******************************************************************************!
 * \fn play
 ******************************************************************************/
nlohmann::json
Server::play()
{
    DEBUG("");
    mPlayer.start();
    nlohmann::json result = mPlayer.currentTitle();
    result["abrev"] = mAbrev;
    return result;
}

/******************************************************************************!
 * \fn pause
 ******************************************************************************/
nlohmann::json
Server::pause()
{
    DEBUG("");
    mPlayer.pause();
    nlohmann::json result = mPlayer.currentTitle();
    result["abrev"] = mAbrev;
    return result;
}

/******************************************************************************!
 * \fn stop
 ******************************************************************************/
nlohmann::json
Server::stop()
{
    DEBUG("");
    auto ms = mPlayer.getPlaytime();
    mList.writeResumeTime(ms);
    mPlayer.stop();
    nlohmann::json result;
    result["result"] = "ok";
    return result;
}

/******************************************************************************!
 * \fn prev
 ******************************************************************************/
nlohmann::json
Server::prev()
{
    DEBUG("");
    mPlayer.startRel(-1);
    nlohmann::json result = mPlayer.currentTitle();
    result["abrev"] = mAbrev;
    return result;
}

/******************************************************************************!
 * \fn next
 ******************************************************************************/
nlohmann::json
Server::next()
{
    DEBUG("");
    mPlayer.startRel(1);
    nlohmann::json result = mPlayer.currentTitle();
    result["abrev"] = mAbrev;
    return result;
}

/******************************************************************************!
 * \fn artist
 ******************************************************************************/
nlohmann::json
Server::artist()
{
    DEBUG("");
    nlohmann::json js = mPlayer.currentTitle();
    return mList.artist(js.value<std::string>("artist", "?"),
                        js.value<std::string>("album", "?"));
}

/******************************************************************************!
 * \fn album
 ******************************************************************************/
nlohmann::json
Server::album(const std::string& artist, int pos)
{
    DEBUG("");
    const auto [path, abrev] = mList.album(artist, pos);
    if (! path.empty()) {
        mPlayer.m3u(path);
        mSelect = path;
        mSelectTime =
            std::chrono::time_point<std::chrono::steady_clock>::min();
        mAbrev = abrev;
        mList.writeLog(path);
    }
    nlohmann::json result = mPlayer.currentTitle();
    if (path.empty()) {
        result["abrev"] = mAbrev;
    } else if (pos != -1) {
        result["cs"] = -1;
    }
    return result;
}

/******************************************************************************!
 * \fn pos
 ******************************************************************************/
nlohmann::json
Server::pos(int pos)
{
    DEBUG("");
    mPlayer.startId(pos);
    nlohmann::json result = mPlayer.currentTitle();
    result["abrev"] = mAbrev;
    return result;
}

/******************************************************************************!
 * \fn dir
 ******************************************************************************/
nlohmann::json
Server::dir(const std::string& path)
{
    auto p = path;
    std::ranges::replace(p, '+', ' ');
    DEBUG(p);
    if (std::filesystem::exists(mPlayer.musicDirectory + '/' +
                                p + "/00.m3u")) {
        mPlayer.m3u(p + "/00.m3u");
        mList.writeLog(p + "/00.m3u");
        return nlohmann::json{};
    } else {
        return mList.dir(p);
    }
}

/******************************************************************************!
 * \fn quit
 ******************************************************************************/
void
Server::quit()
{
    DEBUG("");
    this->loop = false;
    loop.notify_one();
}

/******************************************************************************!
 * \fn musicDirectory
 ******************************************************************************/
const std::string&
Server::musicDirectory()
{
    return mPlayer.musicDirectory;
}

/******************************************************************************!
 * \fn checksum
 ******************************************************************************/
nlohmann::json
Server::checksum()
{
    DEBUG("");
    int cs = 0;
    std::string path =
        mPlayer.musicDirectory + '/' +
        mSelect.substr(0, mSelect.rfind('/') + 1);

    if (std::filesystem::exists(path + ".sha")) {
        auto cmd = std::string("cd \"") + path +
            "\"; /usr/bin/sha1sum -c .sha >/dev/null";
        if (::system(cmd.c_str()) != 0) {
            cs = 1;
            ERROR(cs);
        }
    } else {
        auto cmd = std::string("cd \"") + path +
            "\"; /usr/bin/sha1sum * >.sha";
        if (::system(cmd.c_str()) != 0) {
            cs = 2;
            ERROR(cs);
        }
    }

    nlohmann::json js = mPlayer.currentTitle();
    if (cs) {
        js["cs"] = cs;
    } else {
        js["abrev"] = mAbrev;
    }
    return js;
}
