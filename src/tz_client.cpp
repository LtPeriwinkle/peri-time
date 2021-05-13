#include "tz_client.hpp"
#include "embed_templates.hpp"
#include "sleepy_discord/sleepy_discord.h"
#include <chrono>
#include <date/tz.h>
#include <sqlite3.h>
#include <sstream>

using namespace SleepyDiscord;

void TzBotClient::onMessage(Message message) {
  if (message.startsWith(";tz")) {
    std::vector<std::string> words;
    std::string msg;
    std::stringstream ss(message.content);
    while (ss >> msg) {
      words.push_back(msg);
    }
    msg = "";
    Embed emb = Embed(Embed::Flag::INVALID_EMBED);
    if (words.size() >= 2) {
      Snowflake<User> bot_id = this->getID();
      if (words[1] == "quit" &&
          (message.author.ID.number() == 658861212657909791)) {
        sendMessage(message.channelID, "bye");
        this->quit();
        return;
      } else if (words[1] == "help") {
        emb = Embed(gen_embed("tzbot help", "this will be something eventually",
                              &userCache.at(bot_id)));
      } else if (words[1] == "set") {
        if (words.size() == 3) {
          /* make sqlite call here lmaooo */
          try {
            const date::time_zone *zone =
                date::locate_zone(words[2]);
            auto time =
                date::make_zoned(zone, date::floor<std::chrono::seconds>(
                                           std::chrono::system_clock::now()));
            std::stringstream zonetime;
            zonetime << "current time: " << time;
            std::string query = "INSERT INTO tbl1 (uid, tz) VALUES (?, ?)";
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(userDB.get(), query.c_str(), query.length(), &stmt, nullptr);
            sqlite3_bind_int64(stmt, 1, message.author.ID.number());
            sqlite3_bind_text(stmt, 2, zone->name().c_str(), zone->name().length(), SQLITE_STATIC); 
            int rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            if (rc != SQLITE_DONE) {
              std::string sql_error(sqlite3_errmsg(userDB.get()));
              emb = Embed(err_embed("database error: " + sql_error, &userCache.at(bot_id)));
            } else {
              emb = Embed(gen_embed("Success",
                                    "set to " + words[2] + ", " + zonetime.str(),
                                    &userCache.at(bot_id)));
            }
          } catch (std::runtime_error &error) {
            emb = Embed(err_embed(error.what(), &userCache.at(bot_id)));
          }
        } else {
          emb =
              Embed(err_embed("Not enough arguments!", &userCache.at(bot_id)));
        }
      } else {
        emb = Embed(err_embed("Command not found", &userCache.at(bot_id)));
      }
      sendMessage(message.channelID, msg, emb);
    }
  }
}
void TzBotClient::onServer(Server server) {
  for (User &user : server.members) {
    userCache.insert({user.ID, user});
  }
}
void TzBotClient::onReady(Ready readyData) {
  sqlite3 *db;
  int rc = sqlite3_open("tz.db", &db);
  if (rc) {
    std::cerr << "Could not open tz.db file" << std::endl;
    this->quit();
  }
  userDB = sqlite3_handle{db};
}
