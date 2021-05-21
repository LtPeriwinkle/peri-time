#define SLEEPY_DEFAULT_REQUEST_MODE SleepyDiscord::Async
#include "sleepy_discord/sleepy_discord.h"
#include "tz_client.hpp"
#include <fstream>
#include <iostream>

int main() {
  std::ifstream tok_file(".bot-token");
  std::string bot_token;
  getline(tok_file, bot_token);
  TzBotClient client(bot_token, SleepyDiscord::USER_CONTROLED_THREADS);
  client.run();
}
