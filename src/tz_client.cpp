#include "tz_client.hpp"
#include "embed_templates.hpp"
#include "sleepy_discord/sleepy_discord.h"
#include <chrono>
#include <date/tz.h>
#include <sqlite3.h>
#include <sstream>

using namespace SleepyDiscord;

void TzBotClient::onMessage(Message message) {
  if (message.author.ID != this->getID() && message.startsWith(";tz")) {
    std::vector<std::string> words;
    std::string msg;
    std::stringstream ss(message.content);
    while (ss >> msg) {
      words.push_back(msg);
    }
    msg = "";
    Embed emb = Embed(Embed::Flag::INVALID_EMBED);
    if (words.size() >= 2) {
      User bot_user;
      try {
        bot_user = userCache.at(this->getID());
      } catch (std::out_of_range &e) {
        bot_user = this->getCurrentUser();
      }
      if (words[1] == "quit" &&
          (message.author.ID.number() == 658861212657909791)) {
        sendMessage(message.channelID, "bye");
        this->quit();
        return;
      } else if (words[1] == "help") {
        emb = Embed(gen_embed("tzbot help", "this will be something eventually",
                              &bot_user));
      } else if (words[1] == "set") {
        if (words.size() == 3) {
          try {
            const date::time_zone *zone = date::locate_zone(words[2]);
            auto time =
                date::make_zoned(zone, date::floor<std::chrono::seconds>(
                                           std::chrono::system_clock::now()));
            std::ostringstream zonetime;
            zonetime << "current time: " << time;
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(userDB.get(), set_tz_query.c_str(),
                               set_tz_query.length(), &stmt, nullptr);
            sqlite3_bind_int64(stmt, 1, message.author.ID.number());
            sqlite3_bind_text(stmt, 2, zone->name().c_str(),
                              zone->name().length(), SQLITE_STATIC);
            int rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            if (rc != SQLITE_DONE) {
              std::string sql_error(sqlite3_errmsg(userDB.get()));
              emb = Embed(err_embed("database error: " + sql_error, &bot_user));
            } else {
              emb = Embed(gen_embed(
                  "Success", "set to " + words[2] + ", " + zonetime.str(),
                  &bot_user));
            }
          } catch (std::runtime_error &error) {
            emb = Embed(err_embed(error.what(), &bot_user));
          }
        } else {
          emb = Embed(err_embed("Not enough arguments!", &bot_user));
        }
      } else if (words[1] == "get") {
        if (words.size() == 2) {
          int rc;
          sqlite3_stmt *stmt;
          sqlite3_prepare_v2(userDB.get(), get_tz_query.c_str(),
                             get_tz_query.length(), &stmt, nullptr);
          sqlite3_bind_int64(stmt, 1, message.author.ID.number());
          rc = sqlite3_step(stmt);
          if (rc != SQLITE_ROW) {
            std::string sql_error(sqlite3_errmsg(userDB.get()));
            emb = Embed(err_embed("database error: " + sql_error, &bot_user));
          } else {
            std::string s(
                reinterpret_cast<char const *>(sqlite3_column_text(stmt, 0)));
            emb = Embed(gen_embed("Timezone set by " + message.author.username +
                                      "#" + message.author.discriminator,
                                  s, &bot_user));
          }
          sqlite3_finalize(stmt);
        } else if (words.size() == 3) {
          /*getter for other users will be here*/
        }
      } else {
        emb = Embed(err_embed("Command not found", &bot_user));
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
