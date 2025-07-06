#ifndef NET_RECEIVER_H
#define NET_RECEIVER_H

#include "game.h"
#include "object.h"

class NetReceiver : public Object {
public:
    NetReceiver() {};
    ~NetReceiver() {};

    void step() override;

private:
    std::vector<Game::SpriteRender> sprites;
    Uint64 last_frame_ticks = 0;
};

class NetReceiverFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        return new NetReceiver();
    }
};

#endif
