#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Adafruit_NeoPixel.h>
#include "constants.h"
#include "message_receiver.h"
#include "rgb_values.h"
#include "ArduinoJson.h"

class SystemState
{

    public:
        SystemState();
        void initialize();
        void process_messages();
        void update_on_serial_event();

    protected:
        MessageReceiver message_receiver_;
        Adafruit_NeoPixel neo_pixel_ = Adafruit_NeoPixel(Num_Pixel, Neo_Pin, NEO_GRB + NEO_KHZ800);
        void handle_json_message(JsonObject &json_msg, JsonObject &json_rsp);
        void handle_xset_command(JsonObject &json_msg, JsonObject &json_rsp);
        void handle_iset_command(JsonObject &json_msg, JsonObject &json_rsp);
        void handle_aset_command(JsonObject &json_msg, JsonObject &json_rsp);
        void handle_off_command(JsonObject &json_msg, JsonObject &json_rsp);
        void handle_num_command(JsonObject &json_msg, JsonObject &json_rsp);

        int get_pixel_index(JsonObject &json_msg, JsonObject &json_rsp);
        RGBValues get_rgb_values(JsonObject &json_msg, JsonObject &json_rsp);
        uint8_t get_color(String key, JsonObject &json_msg, JsonObject &json_rsp);

        void all_off();

};

#endif
