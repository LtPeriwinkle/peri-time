#include "embed_templates.hpp"
#include <iostream>
#include <sstream>
#include <string>
std::string gen_embed(std::string title, std::string description,
                      SleepyDiscord::User *sender) {
  std::stringstream ss;
  ss << R"({"title": ")" << title << R"(", "description": ")" << description
     << R"(", "color": )" << 4293174
     << R"(,"footer": {"text": ")" << sender->username << R"(", 
     "icon_url": "https://cdn.discordapp.com/avatars/)"
     << sender->ID.number() << "/" << sender->avatar << ".png"
     << R"("}})";
  return ss.str();
}

std::string err_embed(std::string error, SleepyDiscord::User *sender) {
  std::stringstream ss;
  ss << R"({"title": "Error",)"
     << R"( "description": ")" << error << R"(", "color": )" << 11089976
     << R"(,"footer": {"text": ")" << sender->username << R"(", 
     "icon_url": "https://cdn.discordapp.com/avatars/)"
     << sender->ID.number() << "/" << sender->avatar << ".png"
     << R"("}})";
  return ss.str();
}

std::string help_embed(SleepyDiscord::User *sender) {
    std::stringstream ss;
    ss << R"({
    "title": "Help",
    "color": 21402,
    "footer": {
      "icon_url": "https://cdn.discordapp.com/avatars/)" << sender->ID.number() << "/" << sender->avatar << ".png" << R"(",
      "text": ")" << sender->username << R"("
    },
    "fields": [
      {
        "name": "`;tz help`",
        "value": "Show this message."
      },
      {
        "name": "`;tz set <timezone>`",
        "value": "Set your timezone to <timezone>. Timezone list available [here](https://kevinnovak.github.io/Time-Zone-Picker/)."
      },
      {
        "name": "`;tz get`",
        "value": "Get the timezone that you set."
      },
      {
        "name": "`;tz get <user>`",
        "value": "Get the timezone set by <user>. You can either ping the user or type their full username/full server nick **exactly**."
      },
      {
        "name": "`;tz set <user> <timezone>`",
        "value": "Available only to those with Manage Users permission. Set the timezone for <user> to <timezone>. Timezone list available [here](https://kevinnovak.github.io/Time-Zone-Picker/). You can either ping the user or type their full username/full server nick **exactly**.",
      }
    ]
})";
  return ss.str();
}
