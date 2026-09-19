/******************************************************************************!
 * \file Server.h
 * \author Sebastien Beaugrand
 * \sa http://beaugrand.chez.com/
 * \copyright CeCILL 2.1 Free Software license
 ******************************************************************************/
#pragma once
#include <atomic>
#include <cpphttplibconnector.hpp>
#include <jsonrpccxx/server.hpp>

class List;
class Player;

/******************************************************************************!
 * \class Server
 ******************************************************************************/
class Server : public jsonrpccxx::JsonRpc2Server
{
public:
    Server(List& list, Player& player);
    nlohmann::json list();
    nlohmann::json info();
    nlohmann::json rand();
    nlohmann::json ok();
    nlohmann::json play();
    nlohmann::json pause();
    nlohmann::json stop();
    nlohmann::json prev();
    nlohmann::json next();
    nlohmann::json artist();
    nlohmann::json album(const std::string& artist, int pos);
    nlohmann::json pos(int pos);
    nlohmann::json dir(const std::string& path);
    const std::string& musicDirectory();
    nlohmann::json checksum();
    void quit();
    ~Server();

    std::atomic_bool loop = true;
    List& mList;
    Player& mPlayer;
    std::string mSelect;
    std::chrono::time_point<std::chrono::steady_clock> mSelectTime;
    std::string mAbrev;
};
