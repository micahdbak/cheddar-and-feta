#include "controller.h"
#include "net_agent.h"
#include "net_sender.h"

void NetSender::step() {
    char line[1024];
    std::string controller_msg;

    for (int i = 0; i < NUM_BUTTONS; i++) {
        snprintf(line, sizeof(line), "%d,%d,%d\n", i, local_controller.is_hit_map[(Button)i], local_controller.is_down_map[(Button)i]);
        controller_msg += line;
    }

    snprintf(line, sizeof(line), "%d\n", local_controller.digit);
    controller_msg += line;

    net_agent->send_message(controller_msg);
}
