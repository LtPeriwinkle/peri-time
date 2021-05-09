#include "tz_client.hpp"
#include "sleepy_discord/sleepy_discord.h"

using namespace SleepyDiscord;

void TzBotClient::onMessage(Message message) {
  if (message.startsWith(";cpptest2")) {
    sendMessage(message.channelID, "something");
  }
}
void TzBotClient::onServer(Server server) {
  for (Channel &channel : server.channels) {
    channelCache.insert({channel.ID, channel});
  }
}

