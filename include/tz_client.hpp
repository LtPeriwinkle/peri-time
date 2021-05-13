#include "sleepy_discord/sleepy_discord.h"
#include "database.hpp"
#include <date/tz.h>
#include <sqlite3.h>
#ifndef TZ_CLIENT_H
#define TZ_CLIENT_H
using namespace SleepyDiscord;

class TzBotClient : public DiscordClient {
public:
  using DiscordClient::DiscordClient;
  void onMessage(Message message) override;
  void onServer(Server server) override;
  void onReady(Ready readyData) override;

private:
  std::unordered_map<Snowflake<User>::RawType, User> userCache;
  sqlite3_handle userDB;
};
#endif
