#define SLEEPY_DEFAULT_REQUEST_MODE SleepyDiscord::Async
#include "sleepy_discord/sleepy_discord.h"
#include "tz_client.hpp"
#include <fstream>
#include <iostream>

int main() {
  std::ifstream tokFile(".bot-token");
  std::string botToken;
  getline(tokFile, botToken);
  TzBotClient client(botToken, SleepyDiscord::USER_CONTROLED_THREADS);
  client.run();
}
