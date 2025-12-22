#ifndef NET_RECEIVER_H
#define NET_RECEIVER_H

#include "object.h"

class NetReceiver : public Object {
public:
    NetReceiver() = default;
    ~NetReceiver() = default;

    void step() override;
};

class NetReceiverFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        return new NetReceiver();
    }
};

#endif
