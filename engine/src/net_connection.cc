#include "net_connection.h"

#include <rtc/rtc.h>

#include <iostream>

#define NUM_ICE_SERVERS 2

#define NTS -1

static const char* ICE_SERVERS[] = {"stun:droppr.net:5051",
                                    "turn:droppr:droppr@droppr.net:5051"};

void NetworkConnection::start_collecting(bool offerer) {
  if (this->state != NetworkConnection::State::DEAD) {
    std::cerr << "NetworkConnection::start_collecting fatal error: already "
                 "collecting or alive"
              << std::endl;
    std::exit(1);
  }

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
  config.portRangeBegin = 0;  // unused
  config.portRangeEnd = 0;    // unused
  config.mtu = 0;             // default
  config.maxMessageSize = 0;  // default

  this->pc = rtcCreatePeerConnection(&config);
  if (this->pc < 0) {
    std::cerr << "NetworkConnection::start_collecting fatal error: "
                 "rtcCreatePeerConnection returned error code: "
              << this->pc << std::endl;
    exit(1);
  }
  rtcSetUserPointer(this->pc, this);
  rtcSetLocalCandidateCallback(this->pc, NetworkConnection::on_candidate);
  rtcSetLocalDescriptionCallback(this->pc, NetworkConnection::on_description);
  rtcSetDataChannelCallback(this->pc, NetworkConnection::on_datachannel);
  rtcSetClosedCallback(this->pc, NetworkConnection::on_close);
  rtcSetErrorCallback(this->pc, NetworkConnection::on_error);

  if (offerer) {
    this->dc = rtcCreateDataChannel(this->pc, "messages");
    if (this->dc < 0) {
      std::cerr << "NetworkConnection::start_collecting fatal error: "
                   "rtcCreateDataChannel returned error code: "
                << this->dc << std::endl;
      exit(1);
    }
    rtcSetOpenCallback(this->dc, NetworkConnection::on_open);
    rtcSetClosedCallback(this->dc, NetworkConnection::on_close);
    rtcSetErrorCallback(this->dc, NetworkConnection::on_error);
    rtcSetMessageCallback(this->dc, NetworkConnection::on_message);
  }

  this->state = NetworkConnection::State::COLLECTING;
}

void NetworkConnection::kill() {
  if (this->dc != -1) {
    rtcDelete(this->dc);
    this->dc = -1;
  }

  if (this->pc != -1) {
    rtcDelete(this->pc);
    this->pc = -1;
  }

  this->state = NetworkConnection::State::DEAD;
}

void NetworkConnection::set_remote_description(const std::string& sdp,
                                               const std::string& type) {
  if (this->state != NetworkConnection::COLLECTING || this->pc < 0) {
    std::cerr
        << "NetworkConnection::set_remote_description fatal error: bad state"
        << std::endl;
    std::exit(1);
  }

  rtcSetRemoteDescription(this->pc, sdp.c_str(), type.c_str());
}

std::pair<std::string, std::string> NetworkConnection::get_local_description() {
  std::pair<std::string, std::string> desc = this->description;
  this->description = std::pair<std::string, std::string>("", "");
  return desc;
}

void NetworkConnection::add_candidate(const std::string& cand,
                                      const std::string& mid) {
  if (this->state != NetworkConnection::COLLECTING || this->pc < 0) {
    std::cerr << "NetworkConnection::add_candidate fatal error: bad state"
              << std::endl;
    std::exit(1);
  }

  rtcAddRemoteCandidate(this->pc, cand.c_str(), mid.c_str());
}

std::pair<std::string, std::string> NetworkConnection::get_candidate() {
  if (this->candidates.empty()) {
    return std::pair<std::string, std::string>("", "");
  }

  std::pair<std::string, std::string> candidate = this->candidates.front();
  this->candidates.pop();
  return candidate;
}

void NetworkConnection::send(const std::string& message) {
  if (this->state != NetworkConnection::ALIVE || this->pc < 0) {
    std::cerr << "NetworkConnection::send warning: couldn't send message"
              << std::endl;
    return;
  }

  rtcSendMessage(this->dc, message.c_str(), NTS);
}

std::string NetworkConnection::receive() {
  if (this->messages.empty()) {
    return "";
  }

  std::string msg = this->messages.front();
  this->messages.pop();
  return msg;
}

void NetworkConnection::on_description(int, const char* sdp, const char* type,
                                       void* user_ptr) {
  // std::cout << "NetworkConnection::on_description" << std::endl;
  NetworkConnection* connection = (NetworkConnection*)user_ptr;
  connection->description.first = sdp;
  connection->description.second = type;
}

void NetworkConnection::on_candidate(int, const char* cand, const char* mid,
                                     void* user_ptr) {
  // std::cout << "NetworkConnection::on_candidate" << std::endl;
  NetworkConnection* connection = (NetworkConnection*)user_ptr;
  connection->candidates.push(std::pair<std::string, std::string>(cand, mid));
}

void NetworkConnection::on_datachannel(int, int dc, void* user_ptr) {
  // std::cout << "NetworkConnection::on_datachannel" << std::endl;
  NetworkConnection* connection = (NetworkConnection*)user_ptr;
  connection->dc = dc;
  rtcSetOpenCallback(dc, NetworkConnection::on_open);
  rtcSetClosedCallback(dc, NetworkConnection::on_close);
  rtcSetErrorCallback(dc, NetworkConnection::on_error);
  rtcSetMessageCallback(dc, NetworkConnection::on_message);
  connection->state = NetworkConnection::State::ALIVE;
}

void NetworkConnection::on_open(int, void* user_ptr) {
  // std::cout << "NetworkConnection::on_open" << std::endl;
  NetworkConnection* connection = (NetworkConnection*)user_ptr;
  connection->state = NetworkConnection::State::ALIVE;
}

void NetworkConnection::on_close(int, void* user_ptr) {
  // std::cout << "NetworkConnection::on_close" << std::endl;
  NetworkConnection* connection = (NetworkConnection*)user_ptr;
  connection->kill();
}

void NetworkConnection::on_error(int, const char* error, void* user_ptr) {
  std::cerr << "NetworkConnection::on_error: " << error << std::endl;
  NetworkConnection* connection = (NetworkConnection*)user_ptr;
  connection->kill();

  // overwrite state change to DEAD in kill
  connection->state = NetworkConnection::State::ERROR;
}

void NetworkConnection::on_message(int, const char* message, int size,
                                   void* user_ptr) {
  NetworkConnection* connection = (NetworkConnection*)user_ptr;
  if (size > 0) {
    std::cerr
        << "NetworkConnection::on_message fatal error: received binary message"
        << std::endl;
    std::exit(1);
  }
  connection->messages.push(message);
}
