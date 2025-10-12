#include "controller.h"
#include "game.h"
#include "net_agent.h"
#include "net_receiver.h"

void NetReceiver::step() {
    while (true) {
        std::string controller_msg = net_agent->next_message();
        if (controller_msg == "") break;

        const char *line = controller_msg.c_str();

        for (int i = 0; i < Button::NUM_BUTTONS; i++) {
            int button, is_hit, is_down;
            sscanf(line, "%d,%d,%d\n", &button, &is_hit, &is_down);

            if (is_hit)
                remote_controller.handle_button_hit((Button)button);

            if (is_down) {
                remote_controller.handle_button_down((Button)button);
            } else {
                remote_controller.handle_button_up((Button)button);
            }

            line = next_line(line);
        }

        int digit;
        sscanf(line, "%d\n", &digit);
        remote_controller.digit = digit;
    }
}
