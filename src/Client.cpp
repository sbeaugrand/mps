/******************************************************************************!
 * \file Client.cpp
 * \author Sebastien Beaugrand
 * \sa http://beaugrand.chez.com/
 * \copyright CeCILL 2.1 Free Software license
 ******************************************************************************/
#include <fstream>
#include "Client.h"
#include "log.h"

/******************************************************************************!
 * \fn Client
 ******************************************************************************/
Client::Client(Input& input, Output& output, const std::string& url)
    : mInput(input)
    , mOutput(output)
    , mHttpClient(url, 8383)
    , mJsonClient(mHttpClient, jsonrpccxx::version::v2)
{
    //mHttpClient.SetTimeout(20000);

    std::string tz;
    if (std::ifstream("/etc/timezone") >> tz; tz == "Europe/Paris") {
        mLang = FR;
    } else {
        mLang = EN;
    }
}

/******************************************************************************!
 * \fn Client
 ******************************************************************************/
Client::~Client()
{
    DEBUG("");
}

/******************************************************************************!
 * \fn close
 ******************************************************************************/
void
Client::close()
{
    this->loop = false;
    mInput.hasEvent = true;
    mInput.hasEvent.notify_one();
}

/******************************************************************************!
 * \fn currentTitle
 ******************************************************************************/
void
Client::currentTitle(const nlohmann::json js)
{
    try {
        nlohmann::json error("");
        auto pause = js.value<bool>("pause", false);
        auto pos = js.value<int>("pos", -1) + 1;
        auto length = js.value<int>("length", -1) + 1;
        auto title = js.value<std::string>("title", error);
        auto album = js.value<std::string>("album", error);
        auto artist = js.value<std::string>("artist", error);
        auto date = js.value<std::string>("date", error);
        auto cs = js.value<int>("cs", 0);
        auto abrev = js.value<std::string>("abrev", error);
        int diffmax = -Output::LCD_SHIFT;
        if (mShift > 0) {
            auto size = album.size();
            int diff = size - mShift - Output::LCD_COLS;
            if (diff >= 0) {
                album = album.substr(mShift);
            } else if (size > Output::LCD_COLS) {
                album = album.substr(size - Output::LCD_COLS);
            }
            if (diffmax < diff) {
                diffmax = diff;
            }
        }
        if (mShift > 0) {
            auto size = title.size();
            int diff = size - mShift - Output::LCD_COLS;
            if (diff >= 0) {
                title = title.substr(mShift);
            } else if (size > Output::LCD_COLS) {
                title = title.substr(size - Output::LCD_COLS);
            }
            if (diffmax < diff) {
                diffmax = diff;
            }
        }
        if (mShift > 0 && diffmax <= -Output::LCD_SHIFT) {
            mShift -= Output::LCD_SHIFT;
        }
        std::string extra;
        if (cs < 0) {
            extra = "CS";
            mOutput.csum = true;
        } else if (cs > 0) {
            extra = std::string("e") + std::to_string(cs);
            mOutput.csum = false;
        } else {
            extra = abrev;
        }
        mOutput.write((pause ? "PAUSE " : "") + artist,
                      album,
                      date + ' ' +
                      std::to_string(pos) + '/' + std::to_string(length) +
                      ' ' + extra,
                      title);
        if (cs < 0) {
            this->currentTitle(mJsonClient.CallMethod<nlohmann::json>(
                                   "checksum", nlohmann::json()));
        }
    } catch (jsonrpccxx::JsonRpcException& e) {
        ERROR(e.what());
    } catch (const std::exception& e) {
        ERROR(e.what());
    }
}

/******************************************************************************!
 * \fn currentAlbum
 ******************************************************************************/
void
Client::currentAlbum(const nlohmann::json js)
{
    char dateR[4] = {};
    if (int d = js.value<int>("days", 0); d > 0) {
        int dInit = d;
        if (d > 9) {
            d /= 7;
            if (d > 9) {
                d = dInit / 31;
                if (d > 9) {
                    d /= 12;
                    dateR[2] = (mLang == FR) ? 'A' : 'Y';
                } else {
                    dateR[2] = 'M';
                }
            } else {
                dateR[2] = (mLang == FR) ? 'S' : 'W';
            }
        } else {
            dateR[2] = (mLang == FR) ? 'J' : 'D';
        }
        dateR[1] = '0' + d;
        dateR[0] = ' ';
    }
    mOutput.write(js.value<std::string>("artist", "?"),
                  js.value<std::string>("album", "?"),
                  js.value<std::string>("date", "?"),
                  js.value<std::string>("abrev", "?") + dateR);
}

/******************************************************************************!
 * \fn albumList
 ******************************************************************************/
void
Client::albumList()
{
    auto line1 = mArtist.value<std::string>("artist", "");
    auto album = mArtist["album"];
    auto size = album.size();
    std::string line2;
    std::string line3;
    std::string line4;
    if (mAlbumPos > 0) {
        line2 = album[mAlbumPos - 1];
    }
    line3 = album[mAlbumPos];
    if (mAlbumPos + 1 < size) {
        line4 = album[mAlbumPos + 1];
    }
    try {
        line3.at(4) = '>';
    } catch (std::out_of_range const& e) {
        ERROR(e.what());
    }
    mOutput.write(line1,
                  line2,
                  line3,
                  line4);
}

/******************************************************************************!
 * \fn letters
 ******************************************************************************/
void
Client::letters(int pos)
{
    auto artist = mArtist.value<std::string>("artist", "");
    switch (pos) {
    case 0:
        mOutput.write(artist,
                      "    AE",
                      "FJ  KO  PT",
                      "    UZ");
        break;
    case 1:
        mOutput.write(artist,
                      "   A",
                      "B  C  D",
                      "   E");
        break;
    case 2:
        mOutput.write(artist,
                      "   F",
                      "G  H  I",
                      "   J");
        break;
    case 3:
        mOutput.write(artist,
                      "   K",
                      "L  M  N",
                      "   O");
        break;
    case 4:
        mOutput.write(artist,
                      "   P",
                      "Q  R  S",
                      "   T");
        break;
    case 5:
        mOutput.write(artist,
                      "   U",
                      "V  W  Y",
                      "   Z");
        break;
    }
}

/******************************************************************************!
 * \fn onEvent
 ******************************************************************************/
State
Client::onEvent(const state::Normal& state, const event::Up&)
{
    this->currentTitle(mJsonClient.CallMethod<nlohmann::json>(1, "prev"));
    return state;
}
State
Client::onEvent(const state::Normal& state, const event::Down&)
{
    this->currentTitle(mJsonClient.CallMethod<nlohmann::json>(1, "next"));
    return state;
}
State
Client::onEvent(const state::Normal&, const event::Left&)
{
    try {
        mArtist = mJsonClient.CallMethod<nlohmann::json>(1, "artist");
        int pos = mArtist.value<int>("pos", -1);
        if (pos >= 0) {
            mAlbumPos = pos;
            this->albumList();
        } else {
            return this->onEvent(state::Album{}, event::Left{});
        }
    } catch (jsonrpccxx::JsonRpcException& e) {
        ERROR(e.what());
    }
    return state::Album{};
}
State
Client::onEvent(const state::Normal& state, const event::Right&)
{
    this->currentTitle(mJsonClient.CallMethod<nlohmann::json>(1, "info"));
    return state;
}
State
Client::onEvent(const state::Normal& state, const event::Ok&)
{
    this->currentTitle(mJsonClient.CallMethod<nlohmann::json>(1, "ok"));
    return state;
}
State
Client::onEvent(const state::Normal& state, const event::Setup&)
{
    try {
        this->currentAlbum(mJsonClient.CallMethod<nlohmann::json>(1, "rand"));
    } catch (jsonrpccxx::JsonRpcException& e) {
        ERROR(e.what());
    }
    return state;
}

State
Client::onEvent(const state::Album& state, const event::Up&)
{
    if (mAlbumPos > 0) {
        --mAlbumPos;
    }
    this->albumList();
    return state;
}
State
Client::onEvent(const state::Album& state, const event::Down&)
{
    if (mAlbumPos + 1 < mArtist["album"].size()) {
        ++mAlbumPos;
    }
    this->albumList();
    return state;
}
State
Client::onEvent(const state::Album&, const event::Left&)
{
    mArtist["artist"] = "";
    mArtistPos = 0;
    this->letters(mArtistPos);
    return state::Artist{};
}
State
Client::onEvent(const state::Album&, const event::Right&)
{
    mShift = 0;
    this->currentTitle(mJsonClient.CallMethod<nlohmann::json>(1, "info"));
    return state::Normal{};
}
State
Client::onEvent(const state::Album&, const event::Ok&)
{
    try {
        nlohmann::json params;
        params["artist"] = mArtist["artist"];
        params["pos"] = mAlbumPos;
        this->currentTitle(mJsonClient.CallMethod<nlohmann::json>(
                               1, "album", params));
    } catch (jsonrpccxx::JsonRpcException& e) {
        ERROR(e.what());
    }
    return state::Normal{};
}
State
Client::onEvent(const state::Album& state, const event::Setup&)
{
    return state;
}

State
Client::onEvent(const state::Artist& state, const event::Up&)
{
    if (mArtistPos == 0) {
        mArtistPos = 1;
    } else {
        mArtist["artist"] = mArtist.value<std::string>("artist", "") +
            mLetterList.at(mArtistPos - 1);
        mArtistPos = 0;
    }
    this->letters(mArtistPos);
    return state;
}
State
Client::onEvent(const state::Artist& state, const event::Down&)
{
    if (mArtistPos == 0) {
        mArtistPos = 5;
    } else {
        mArtist["artist"] = mArtist.value<std::string>("artist", "") +
            mLetterList.at(mArtistPos - 1 + 20);
        mArtistPos = 0;
    }
    this->letters(mArtistPos);
    return state;
}
State
Client::onEvent(const state::Artist& state, const event::Left&)
{
    if (mArtistPos == 0) {
        mArtistPos = 2;
    } else {
        mArtist["artist"] = mArtist.value<std::string>("artist", "") +
            mLetterList.at(mArtistPos - 1 + 5);
        mArtistPos = 0;
    }
    this->letters(mArtistPos);
    return state;
}
State
Client::onEvent(const state::Artist& state, const event::Right&)
{
    mShift = 0;
    if (mArtistPos == 0) {
        mArtistPos = 4;
    } else {
        mArtist["artist"] = mArtist.value<std::string>("artist", "") +
            mLetterList.at(mArtistPos - 1 + 15);
        mArtistPos = 0;
    }
    this->letters(mArtistPos);
    return state;
}
State
Client::onEvent(const state::Artist& state, const event::Ok&)
{
    if (mArtistPos == 0) {
        mArtistPos = 3;
    } else {
        mArtist["artist"] = mArtist.value<std::string>("artist", "") +
            mLetterList.at(mArtistPos - 1 + 10);
        mArtistPos = 0;
    }
    this->letters(mArtistPos);
    return state;
}
State
Client::onEvent(const state::Artist&, const event::Setup&)
{
    try {
        nlohmann::json params;
        params["artist"] = mArtist["artist"];
        params["pos"] = -1;
        mJsonClient.CallMethod<nlohmann::json>(1, "album", params);
    } catch (jsonrpccxx::JsonRpcException& e) {
        ERROR(e.what());
    }
    return this->onEvent(state::Normal{}, event::Left{});
}

/******************************************************************************!
 * \fn processEvent
 ******************************************************************************/
void
Client::processEvent(const Event& event)
{
    state = std::visit(
        [this](const auto& st, const auto& ev) {
        return this->onEvent(st, ev);
    },
        state, event);
}

/******************************************************************************!
 * \fn run
 ******************************************************************************/
int
Client::run()
{
    for (;;) {
        try {
            auto d = mJsonClient.CallMethod<std::string>(1, "musicDirectory");
            if (! d.empty()) {
                DEBUG(d);
                mOutput.musicDirectory = d;
            }
            break;
        } catch (jsonrpccxx::JsonRpcException& e) {
            ERROR(e.what());
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } catch (const std::exception& e) {
            ERROR(e.what());
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    this->currentTitle(mJsonClient.CallMethod<nlohmann::json>(1, "info"));
    while (this->loop) {
        mInput.hasEvent.wait(false);
        if (! this->loop) {
            return 0;
        }
        auto key = mInput.key;
        mInput.hasEvent = false;
        switch (key) {
        case Input::KEY_UP:
            mShift = 0;
            this->processEvent(event::Up{});
            break;
        case Input::KEY_DOWN:
            mShift = 0;
            this->processEvent(event::Down{});
            break;
        case Input::KEY_LEFT:
            if (mShift > 0) {
                mShift -= Output::LCD_SHIFT;
                this->processEvent(event::Right{});
            } else {
                mShift = 0;
                this->processEvent(event::Left{});
            }
            break;
        case Input::KEY_RIGHT:
            mShift += Output::LCD_SHIFT;
            this->processEvent(event::Right{});
            break;
        case Input::KEY_OK:
            mShift = 0;
            this->processEvent(event::Ok{});
            break;
        case Input::KEY_SETUP:
            mShift = 0;
            this->processEvent(event::Setup{});
            break;
        case Input::KEY_BACK:
            mShift = 0;
            this->currentTitle(mJsonClient.CallMethod<nlohmann::json>(
                                   1, "info"));
            break;
        case Input::KEY_UNDEFINED:
            break;
        }
    }
    return 0;
}
