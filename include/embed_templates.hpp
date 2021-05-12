#include "sleepy_discord/sleepy_discord.h"
#include <string>
#ifndef EMBED_TEMPLATES_H
#define EMBED_TEMPLATES_H
std::string gen_embed(std::string title, std::string description,
                      SleepyDiscord::User *sender);
std::string err_embed(std::string error, SleepyDiscord::User *sender);
#endif
