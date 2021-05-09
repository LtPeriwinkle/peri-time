#include "sleepy_discord/sleepy_discord.h"
#ifndef TZ_CLIENT_H
#define TZ_CLIENT_H
using namespace SleepyDiscord;

class TzBotClient : public DiscordClient {
public:
  using DiscordClient::DiscordClient;
  void onMessage(Message message) override;
  void onServer(Server server) override;

private:
  std::unordered_map<Snowflake<Channel>::RawType, Channel> channelCache;
};
#endif // TZ_CLIENT_H
