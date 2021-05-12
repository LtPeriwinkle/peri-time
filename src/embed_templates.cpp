#include "embed_templates.hpp"
#include <iostream>
#include <sstream>
#include <string>
std::string gen_embed(std::string title, std::string description,
                      SleepyDiscord::User *sender) {
  std::stringstream ss;
  ss << R"({"title": ")" << title << R"(", "description": ")" << description
     << R"(", "color": )" << 4293174
     << R"(,"footer": {"text": "tzbot", "icon_url": "https://cdn.discordapp.com/avatars/)"
     << sender->ID.number() << "/" << sender->avatar << ".png"
     << R"("}})";
  return ss.str();
}

std::string err_embed(std::string error, SleepyDiscord::User *sender) {
  std::stringstream ss;
  ss << R"({"title": "Error",)"
     << R"( "description": ")" << error << R"(", "color": )" << 11089976
     << R"(,"footer": {"text": "tzbot", "icon_url": "https://cdn.discordapp.com/avatars/)"
     << sender->ID.number() << "/" << sender->avatar << ".png"
     << R"("}})";
  return ss.str();
}
