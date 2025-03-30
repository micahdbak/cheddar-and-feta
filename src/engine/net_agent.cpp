#include "net_agent.h"

#include <rtc/rtc.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <random>

#define SIGNALLING_URL "ws://localhost:8080/sc/"

#define ICE_SERVERS (const char *[]){\
    "stun:stun.l.google.com:19302",\
    "stun:stun1.l.google.com:19302",\
    "stun:stun2.l.google.com:19302",\
    "stun:stun3.l.google.com:19302"\
}
#define NUM_ICE_SERVERS 4

#define CANDIDATE_MSG   "candidate"
#define DESCRIPTION_MSG "description"
#define FAIL_MSG        "fail"
#define PING_MSG        "ping"

#define MSG_IS_NTS -1

NetworkAgent *net_agent = nullptr;

NetworkAgent::NetworkAgent(bool offerer) {
    if (net_agent != nullptr) {
        std::cerr << "NetworkAgent::NetworkAgent: a network agent already exists." << std::endl;
        std::exit(1);
    }

    net_agent = this;
    this->offerer = offerer;
    rtcPreload();

    // start message loop
    this->message_thread = new std::thread(NetworkAgent::message_loop);
}

NetworkAgent::~NetworkAgent() {
    {
        std::lock_guard<std::mutex> guard(this->stop_message_loop_mutex);
        this->stop_message_loop = true;
    }
    this->push_message("stop"); // if connected, will break the waiting
    this->message_thread->join();
    this->message_thread = nullptr;
    rtcCleanup();
}

static std::string make_code() {
    static char charset[] = "ABCEFHJKLMNPQRTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset)-2);
    
    std::string ret;
    for (int i = 0; i < 6; i++) {
        char c = charset[dis(gen)];
        ret.push_back(c);
    }

    return ret;
}

void NetworkAgent::initiate() {
    if (!net_agent->initiated) {
        std::cout << "Initiating!" << std::endl;
        net_agent->initiated = true;

        rtcConfiguration config;
        config.iceServers = ICE_SERVERS;
        config.iceServersCount = NUM_ICE_SERVERS;
        config.proxyServer = nullptr;
        config.bindAddress = nullptr;
        config.certificateType = RTC_CERTIFICATE_DEFAULT;
        config.iceTransportPolicy = RTC_TRANSPORT_POLICY_ALL;
        config.enableIceTcp = true;
        config.enableIceUdpMux = false;
        config.disableAutoNegotiation = false;
        config.forceMediaTransport = false;
        config.portRangeBegin = 0; // unused
        config.portRangeEnd = 0; // unused
        config.mtu = 0; // default
        config.maxMessageSize = 0; // default

        net_agent->pc = rtcCreatePeerConnection(&config);
        if (net_agent->pc < 0) {
            std::cerr << "Couldn't create peer connection: " << net_agent->pc << std::endl;
            exit(1);
        }
        rtcSetLocalCandidateCallback(net_agent->pc, NetworkAgent::candidate_cb);
        rtcSetLocalDescriptionCallback(net_agent->pc, NetworkAgent::description_cb);
        rtcSetDataChannelCallback(net_agent->pc, NetworkAgent::datachannel_cb);
        rtcSetOpenCallback(net_agent->pc, NetworkAgent::open_cb);
        rtcSetClosedCallback(net_agent->pc, NetworkAgent::close_cb);
        rtcSetErrorCallback(net_agent->pc, NetworkAgent::error_cb);

        if (net_agent->offerer) {
            net_agent->dc = rtcCreateDataChannel(net_agent->pc, "messages");
            if (net_agent->dc < 0) {
                std::cerr << "Couldn't create data channel: " << net_agent->dc << std::endl;
                exit(1);
            }
            rtcSetMessageCallback(net_agent->dc, NetworkAgent::message_cb);
            rtcSetOpenCallback(net_agent->dc, NetworkAgent::open_cb);
            rtcSetClosedCallback(net_agent->dc, NetworkAgent::close_cb);
            rtcSetErrorCallback(net_agent->dc, NetworkAgent::error_cb);
        }
    }
}

void NetworkAgent::reset(bool delete_ws) {
    if (delete_ws) {
        net_agent->set_state(NetworkAgent::State::NO_CONNECTION);
    }

    if (net_agent->initiated) {
        std::cout << "Resetting!" << std::endl;
        net_agent->initiated = false;
        if (delete_ws && net_agent->ws >= 0) {
            rtcDelete(net_agent->ws);
            net_agent->ws = -1;
        }
        if (net_agent->pc >= 0) {
            rtcDelete(net_agent->pc);
            net_agent->pc = -1;
        }
        if (net_agent->dc >= 0) {
            rtcDelete(net_agent->dc);
            net_agent->dc = -1;
        }
    }
}

bool NetworkAgent::next_ws_message(std::string &str) {
    int rtc_err;
    do {
        int size;
        rtc_err = rtcReceiveMessage(net_agent->ws, NULL, &size);

        if (rtc_err == RTC_ERR_SUCCESS && size < 0) {
            char *buffer = new char[size * -1];
            rtcReceiveMessage(net_agent->ws, buffer, &size);
            str = buffer;
            delete[] buffer;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } while (rtc_err == RTC_ERR_NOT_AVAIL);

    return rtc_err == RTC_ERR_SUCCESS;
}

void NetworkAgent::description_cb(int, const char *sdp, const char *type, void *) {
    std::cout << "description" << std::endl;
    if (net_agent->ws < 0) {
        return;
    }

    if (rtcSendMessage(net_agent->ws, DESCRIPTION_MSG, MSG_IS_NTS) != RTC_ERR_SUCCESS ||
        rtcSendMessage(net_agent->ws, sdp, MSG_IS_NTS) != RTC_ERR_SUCCESS ||
        rtcSendMessage(net_agent->ws, type, MSG_IS_NTS) != RTC_ERR_SUCCESS) {
        NetworkAgent::reset(true);
        return;
    }
}

void NetworkAgent::candidate_cb(int, const char *cand, const char *mid, void *) {
    std::cout << "candidate" << std::endl;
    if (net_agent->ws < 0) {
        return;
    }

    if (rtcSendMessage(net_agent->ws, CANDIDATE_MSG, MSG_IS_NTS) != RTC_ERR_SUCCESS ||
        rtcSendMessage(net_agent->ws, cand, MSG_IS_NTS) != RTC_ERR_SUCCESS ||
        rtcSendMessage(net_agent->ws, mid, MSG_IS_NTS) != RTC_ERR_SUCCESS) {
        NetworkAgent::reset(true);
        return;
    }
}

void NetworkAgent::datachannel_cb(int, int dc, void *) {
    if (net_agent->dc >= 0) {
        std::cerr << "A second datachannel was just attempted." << std::endl;
        std::exit(1);
    }

    net_agent->dc = dc;
    rtcSetMessageCallback(dc, NetworkAgent::message_cb);
    rtcSetOpenCallback(dc, NetworkAgent::open_cb);
    rtcSetClosedCallback(dc, NetworkAgent::close_cb);
    rtcSetErrorCallback(dc, NetworkAgent::error_cb);
}

void NetworkAgent::message_cb(int, const char *message, int size, void *) {
    if (size >= 0) {
        std::cerr << "A binary message was just sent." << std::endl;
        std::exit(1);
    }

    std::string msg = message;
    net_agent->push_message(msg);
}

void NetworkAgent::error_cb(int id, const char *error, void *) {
    std::string src = "unknown";
    if (id == net_agent->ws) src = "signalling server";
    if (id == net_agent->pc) src = "peer connection";
    if (id == net_agent->dc) src = "data channel";

    std::cerr << "Error from " << src << ": " << error << std::endl;
    NetworkAgent::reset(true); // start from scratch
}

void NetworkAgent::open_cb(int id, void *) {
    // connected
    if (id == net_agent->dc && net_agent->ws >= 0) {
        rtcDelete(net_agent->ws);
        net_agent->ws = -1;
        net_agent->set_state(NetworkAgent::State::CONNECTED);
    }

    std::string src = "unknown";
    if (id == net_agent->ws) src = "signalling server";
    if (id == net_agent->pc) src = "peer connection";
    if (id == net_agent->dc) src = "data channel";

    std::cout << "Opened " << src << std::endl;
}

void NetworkAgent::close_cb(int id, void *) {
    if (net_agent->initiated && (id == net_agent->dc || id == net_agent->pc)) {
        NetworkAgent::reset(true);
        net_agent->set_state(NetworkAgent::State::NO_CONNECTION);
    }

    std::string src = "unknown";
    if (id == net_agent->ws) {
        src = "signalling server";
        rtcDelete(net_agent->ws);
        net_agent->ws = -1;
    }
    if (id == net_agent->pc) src = "peer connection";
    if (id == net_agent->dc) src = "data channel";

    std::cout << "Closed " << src << std::endl;
}

void NetworkAgent::no_connection() {
    std::string connection_code;

    // get connection code either from user or generate
    if (net_agent->offerer) {
        connection_code = net_agent->get_connection_code();

        if (connection_code.empty()) {
            // wait 500ms and check again
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            return;
        }
    } else {
        connection_code = make_code();
    }

    // attempt to connect using connection code
    std::string url = SIGNALLING_URL + connection_code;
    net_agent->ws = rtcCreateWebSocket(url.c_str());

    if (net_agent->ws >= 0) {
        net_agent->set_state(NetworkAgent::State::WAITING_FOR_PEER);

        // websocket server should send a code
        if (!net_agent->offerer) {
            net_agent->set_connection_code(connection_code);
        }

        rtcSetOpenCallback(net_agent->ws, NetworkAgent::open_cb);
        rtcSetClosedCallback(net_agent->ws, NetworkAgent::close_cb);
        rtcSetErrorCallback(net_agent->ws, NetworkAgent::error_cb);
    }
}

void NetworkAgent::waiting_for_peer() {
    int size = 0;
    int rtc_err = rtcReceiveMessage(net_agent->ws, NULL, &size);

    if (rtc_err == RTC_ERR_SUCCESS && size < 0) {
        std::string msg;
        char *buffer = new char[size * -1];
        rtcReceiveMessage(net_agent->ws, buffer, &size);
        msg = buffer;
        delete[] buffer;

        // message from peer! initiate!
        if (msg != FAIL_MSG && !net_agent->initiated) {
            // will prepare but only if not initiated already
            NetworkAgent::initiate();
        }

        if (msg == CANDIDATE_MSG) {
            std::string candidate, mid;
            if (!NetworkAgent::next_ws_message(candidate) ||
                !NetworkAgent::next_ws_message(mid)) {
                NetworkAgent::reset(true);
            } else {
                rtcAddRemoteCandidate(net_agent->pc, candidate.c_str(), mid.c_str());
                std::cout << "got candidate" << std::endl;
                return; // no sleeping
            }
        } else if (msg == DESCRIPTION_MSG) {
            std::string description, type;
            if (!NetworkAgent::next_ws_message(description) ||
                !NetworkAgent::next_ws_message(type)) {
                NetworkAgent::reset(true);
            } else {
                rtcSetRemoteDescription(net_agent->pc, description.c_str(), type.c_str());
                std::cout << "got description" << std::endl;
                return; // no sleeping
            }
        } else if (msg == FAIL_MSG) {
            std::cout << "got fail" << std::endl;
            // if was negotionating, reset
            NetworkAgent::reset(false);
        } else if (msg == PING_MSG) {
            std::cout << "got ping" << std::endl;
        } else {
            std::cerr << "Unexpected message from the WebSocket server: " << msg << std::endl;
            std::exit(1);
        }
    } else if (rtc_err == RTC_ERR_NOT_AVAIL) {
        if (!net_agent->initiated && !net_agent->offerer) {
            std::cout << "sent ping" << std::endl;
            rtcSendMessage(net_agent->ws, PING_MSG, MSG_IS_NTS);
        }
    } else {
        // an unexpected error occurred; reset everything and reconnect
        NetworkAgent::reset(true);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void NetworkAgent::connected() {
    // wait until the send messages queue has items in it
    std::lock_guard<std::mutex> guard(net_agent->send_messages_mutex);
    // net_agent->cv.wait(lk, []{ return !net_agent->send_messages.empty(); });

    while (!net_agent->send_messages.empty()) {
        std::string msg = net_agent->send_messages.front();
        net_agent->send_messages.pop();

        if (net_agent->dc >= 0) {
            rtcSendMessage(net_agent->dc, msg.c_str(), MSG_IS_NTS);
        }
    }
}

void NetworkAgent::message_loop() {
    while (!net_agent->should_stop()) {
        switch (net_agent->get_state()) {
        case NetworkAgent::State::NO_CONNECTION:
            NetworkAgent::no_connection();
            break;

        case NetworkAgent::State::WAITING_FOR_PEER:
            NetworkAgent::waiting_for_peer();
            break;

        case NetworkAgent::State::CONNECTED:
            NetworkAgent::connected();
            break;

        default: break;
        }
    }

    if (net_agent->ws >= 0) {
        rtcDelete(net_agent->ws);
        net_agent->ws = -1;
    }
    if (net_agent->dc >= 0) {
        rtcDelete(net_agent->dc);
        net_agent->dc = -1;
    }
    if (net_agent->pc >= 0) {
        rtcDelete(net_agent->pc);
        net_agent->pc = -1;
    }
}
