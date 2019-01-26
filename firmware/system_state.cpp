#include <Streaming.h>
#include "system_state.h"


SystemState::SystemState() 
{}


void SystemState::initialize()
{
    Serial.begin(Baudrate);
    neo_pixel_.begin(); 
    all_off();
}


void SystemState::process_messages()
{
    if (message_receiver_.available())
    {
        String msg = message_receiver_.next();

        StaticJsonBuffer<Json_Message_Buffer_Size> json_msg_buffer;
        StaticJsonBuffer<Json_Message_Buffer_Size> json_rsp_buffer;

        JsonObject &json_msg = json_msg_buffer.parse(msg);
        JsonObject &json_rsp = json_rsp_buffer.createObject();

        if (json_msg.success())
        {
            handle_json_message(json_msg, json_rsp);
        }
        else
        {
            json_rsp["ok"] = false;
            json_rsp["msg"] = "parse error";
        }

        if (!json_rsp["ok"])
        {
            // What to do on failure - set leds off??
        }

        json_rsp.printTo(Serial);
        Serial << endl;
    }
}


void SystemState::update_on_serial_event()
{
    message_receiver_.read_data();
}


void SystemState::handle_json_message(JsonObject &json_msg, JsonObject &json_rsp)
{
    String command = json_msg["cmd"] | "missing";

    if (command.equals("missing"))
    {
        json_rsp["ok"] = false;
        json_rsp["msg"] = "cmd key missing";
    } 
    else if (command.equals("xset"))
    {
        handle_xset_command(json_msg, json_rsp);
    }
    else if (command.equals("iset"))
    {
        handle_iset_command(json_msg, json_rsp);
    }
    else if (command.equals("aset"))
    {
        handle_aset_command(json_msg, json_rsp);
    }
    else if (command.equals("off"))
    {
        handle_off_command(json_msg, json_rsp);
    }
    else if (command.equals("num"))
    {
        handle_num_command(json_msg, json_rsp);
    }
    else
    {
        json_rsp["ok"] = false;
        json_rsp["msg"] = "unknown command";
    }
} 


void SystemState::handle_xset_command(JsonObject &json_msg, JsonObject &json_rsp)
{ 
    // Set led value exclusive - all other values set to zero. 
    json_rsp["ok"] = true;
    json_rsp["msg"] = "";

    int index = get_pixel_index(json_msg, json_rsp);
    if (!json_rsp["ok"])
    {
        return;
    }

    RGBValues rgb_values = get_rgb_values(json_msg,json_rsp);
    if (!json_rsp["ok"])
    {
        return;
    }

    neo_pixel_.setPixelColor(index, neo_pixel_.Color(rgb_values.red,rgb_values.green,rgb_values.blue));
    for (int i=0; i<Num_Pixel; i++)
    {
        if (i != index)
        {
            neo_pixel_.setPixelColor(i, neo_pixel_.Color(0,0,0));
        }
    }
    neo_pixel_.show();
}


void SystemState::handle_iset_command(JsonObject &json_msg, JsonObject &json_rsp)
{
    // Set led value inclusive - other keep there value
    json_rsp["ok"] = true;
    json_rsp["msg"] = "";

    int index = get_pixel_index(json_msg, json_rsp);
    if (!json_rsp["ok"])
    {
        return;
    }

    RGBValues rgb_values = get_rgb_values(json_msg,json_rsp);
    if (!json_rsp["ok"])
    {
        return;
    }

    neo_pixel_.setPixelColor(index, neo_pixel_.Color(rgb_values.red,rgb_values.green,rgb_values.blue));
    neo_pixel_.show();
}


void SystemState::handle_aset_command(JsonObject &json_msg, JsonObject &json_rsp)
{
    // Set all leds to the same values
    json_rsp["ok"] = true;
    json_rsp["msg"] = "";

    RGBValues rgb_values = get_rgb_values(json_msg,json_rsp);
    if (!json_rsp["ok"])
    {
        return;
    }

    for (int i=0; i<Num_Pixel; i++)
    {
        neo_pixel_.setPixelColor(i, neo_pixel_.Color(rgb_values.red,rgb_values.green,rgb_values.blue));
    }
    neo_pixel_.show();
}


void SystemState::handle_off_command(JsonObject &json_msg, JsonObject &json_rsp)
{
    json_rsp["ok"] = true;
    json_rsp["msg"] = "";
    all_off();
}


void SystemState::handle_num_command(JsonObject &json_msg, JsonObject &json_rsp)
{
    json_rsp["ok"] = true;
    json_rsp["msg"] = "";
    json_rsp["num"] = Num_Pixel;
}


int SystemState::get_pixel_index(JsonObject &json_msg, JsonObject &json_rsp)
{
    JsonVariant index_var = json_msg["i"];
    if (!index_var.success())
    {
        json_rsp["ok"] = false;
        json_rsp["msg"] = "missing key 'value'";
        return;
    }
    if (!index_var.is<int>())
    {
        json_rsp["ok"] = false;
        json_rsp["msg"] = "led index must be int";
        return;
    }
    int index = index_var.as<int>();
    if ((index < 0) || (index >= Num_Pixel))
    {
        json_rsp["ok"] = false;
        json_rsp["msg"] = "led index out of range";
        return;
    }
    return index;
}


RGBValues SystemState::get_rgb_values(JsonObject &json_msg, JsonObject &json_rsp)
{

    json_rsp["ok"] = true;
    json_rsp["msg"] = "";
    RGBValues rgb_values = {0,0,0};

    rgb_values.red = get_color(String("r"), json_msg, json_rsp);
    if (!json_rsp["ok"])
    {
        return rgb_values;
    }

    rgb_values.green = get_color(String("g"), json_msg, json_rsp);
    if (!json_rsp["ok"])
    {
        return rgb_values;
    }

    rgb_values.blue = get_color(String("b"), json_msg, json_rsp);
    if (!json_rsp["ok"])
    {
        return rgb_values;
    }

    return rgb_values;
}

uint8_t SystemState::get_color(String key, JsonObject &json_msg, JsonObject &json_rsp)
{
    JsonVariant color_var = json_msg[key];
    if (!color_var.success())
    {
        json_rsp["ok"] = false;
        json_rsp["msg"] = String("missing key: ") + key;
        return 0;
    }

    if (!color_var.is<int>())
    {
        json_rsp["ok"] = false;
        json_rsp["msg"] = "led index must be int";
        return 0;
    }

    int color_int = color_var.as<int>();
    if ((color_int <0) || (color_int > 255))
    {
        json_rsp["ok"] = false;
        json_rsp["msg"] = "color value must in [0,255]";
        return 0; 
    }
    return uint8_t(color_int);
}


void SystemState::all_off()
{
    for (int j=0; j<Num_Pixel; j++)
    {
        neo_pixel_.setPixelColor(j, neo_pixel_.Color(0,0,0));
    }
    neo_pixel_.show();
}
