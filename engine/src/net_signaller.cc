#include "net_signaller.h"

#include <rtc/rtc.h>

#include <chrono>
#include <iostream>
#include <thread>

#define SIGNALLING_URL "wss://cheese.micahdb.com/sc/"

#define NTS -1

void NetworkSignaller::connect(const std::string& code, bool offerer) {
  if (this->state != NetworkSignaller::State::NOT_CONNECTED) {
    std::cerr << "NetworkSignaller::connect fatal error: already connecting or "
                 "connected"
              << std::endl;
    std::exit(1);
  }

  std::string url = SIGNALLING_URL + code;
  this->ws = rtcCreateWebSocket(url.c_str());
  if (this->ws < 0) {
    std::cerr << "NetworkSignaller::connect fatal error: rtcCreateWebSocket "
                 "returned error code: "
              << this->ws << std::endl;
    std::exit(1);
  }

  rtcSetUserPointer(this->ws, this);
  rtcSetOpenCallback(this->ws, NetworkSignaller::on_open);
  rtcSetClosedCallback(this->ws, NetworkSignaller::on_close);
  rtcSetErrorCallback(this->ws, NetworkSignaller::on_error);
  rtcSetMessageCallback(this->ws, NetworkSignaller::on_message);

  this->state = NetworkSignaller::State::CONNECTING;
}

void NetworkSignaller::disconnect() {
  if (this->ws != -1) {
    rtcDelete(this->ws);
    this->ws = -1;
  }

  this->state = NetworkSignaller::State::NOT_CONNECTED;
}

void NetworkSignaller::send(const std::string& message) {
  if (this->state != NetworkSignaller::State::CONNECTED || this->ws < 0) {
    std::cerr << "NetworkSignaller::send fatal error: not connected"
              << std::endl;
    std::exit(1);
  }

  rtcSendMessage(this->ws, message.c_str(), NTS);
}

std::string NetworkSignaller::receive() {
  if (this->messages.empty()) {
    return "";
  }

  std::string msg = this->messages.front();
  this->messages.pop();
  return msg;
}

std::string NetworkSignaller::block_receive() {
  // block up to a full second: 10 * 100ms = 1s
  for (int i = 0; i < 10; i++) {
    std::string msg = this->receive();

    if (!msg.empty()) {
      return msg;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return "";
}

void NetworkSignaller::on_open(int, void* user_ptr) {
  // std::cout << "NetworkSignaller::on_open" << std::endl;
  NetworkSignaller* signaller = (NetworkSignaller*)user_ptr;
  signaller->state = NetworkSignaller::State::CONNECTED;
}

void NetworkSignaller::on_close(int, void* user_ptr) {
  // std::cout << "NetworkSignaller::on_close" << std::endl;
  NetworkSignaller* signaller = (NetworkSignaller*)user_ptr;
  signaller->disconnect();
}

void NetworkSignaller::on_error(int, const char* error, void* user_ptr) {
  std::cerr << "NetworkSignaller::on_error: " << error << std::endl;
  NetworkSignaller* signaller = (NetworkSignaller*)user_ptr;
  signaller->disconnect();
}

void NetworkSignaller::on_message(int, const char* message, int size,
                                  void* user_ptr) {
  // std::cout << "NetworkSignaller::on_message" << std::endl;
  NetworkSignaller* signaller = (NetworkSignaller*)user_ptr;
  signaller->messages.push(message);
}
