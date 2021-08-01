#include "sleepy_discord/sleepy_discord.h"
#include "database.hpp"
#include <date/tz.h>
#include <sqlite3.h>
#ifndef TZ_CLIENT_H
#define TZ_CLIENT_H

class TzBotClient : public SleepyDiscord::DiscordClient {
public:
  using SleepyDiscord::DiscordClient::DiscordClient;
  void onMessage(SleepyDiscord::Message message) override;
  void onServer(SleepyDiscord::Server server) override;
  void onReady(SleepyDiscord::Ready readyData) override;

private:
  std::string doCommand(SleepyDiscord::Message *message);
  std::string tzGet(std::vector<std::string> words, SleepyDiscord::Message *message);

  std::unordered_map<SleepyDiscord::Snowflake<SleepyDiscord::User>::RawType, SleepyDiscord::User> userCache;
  std::unordered_map<SleepyDiscord::Snowflake<SleepyDiscord::Server>::RawType, SleepyDiscord::Server> guildCache;
  std::string get_tz_query = "SELECT tz FROM tbl1 WHERE uid=?;";
  std::string set_tz_query = "INSERT OR REPLACE INTO tbl1(uid, tz) VALUES(?, ?);";
  sqlite3_handle userDB;
};
#endif
