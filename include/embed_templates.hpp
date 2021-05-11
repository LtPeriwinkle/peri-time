#include <string>
#ifndef EMBED_TEMPLATES_H
#define EMBED_TEMPLATES_H
std::string gen_embed(std::string title, std::string description, int color);

static const std::string err_embed = R"({"title": "Unknown command", "description": "Try ;tz help for a list of valid commands", "color": 11089976})";
#endif
