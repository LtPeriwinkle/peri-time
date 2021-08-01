#include "tz_client.hpp"
#include "embed_templates.hpp"
#include "sleepy_discord/sleepy_discord.h"
#include <chrono>
#include <date/tz.h>
#include <regex>
#include <sqlite3.h>
#include <sstream>

using namespace SleepyDiscord;

static const std::regex pong = std::regex("<@!?[0-9]{18,}>");

void TzBotClient::onMessage(Message message) {
    if (message.author.ID != this->getID() && message.startsWith(";tz")) {
        std::vector<std::string> words;
        std::string msg;
        std::stringstream ss(message.content);
        while (ss >> msg) {
            words.push_back(msg);
        }
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
            } else if (words[1] == "list") {
                emb = Embed(gen_embed(
                    "Timezone List",
                    "All available timezones and their locations can "
                    "be found at https://kevinnovak.github.io/Time-Zone-Picker",
                    &bot_user));
            } else if (words[1] == "help") {
                emb = Embed(help_embed(&bot_user));
            } else if (words[1] == "set") {
                if (words.size() == 3) {
                    try {
                        const date::time_zone *zone =
                            date::locate_zone(words[2]);
                        auto time = date::make_zoned(
                            zone, date::floor<std::chrono::seconds>(
                                      std::chrono::system_clock::now()));
                        std::ostringstream zonetime;
                        zonetime << "current time: " << time;
                        sqlite3_stmt *stmt;
                        sqlite3_prepare_v2(userDB.get(), set_tz_query.c_str(),
                                           set_tz_query.length(), &stmt,
                                           nullptr);
                        sqlite3_bind_int64(stmt, 1, message.author.ID.number());
                        sqlite3_bind_text(stmt, 2, zone->name().c_str(),
                                          zone->name().length(), SQLITE_STATIC);
                        int rc = sqlite3_step(stmt);
                        sqlite3_finalize(stmt);
                        if (rc != SQLITE_DONE) {
                            std::string sql_error(sqlite3_errmsg(userDB.get()));
                            emb =
                                Embed(err_embed(std::string("database error: ")
                                                    .append(sql_error),
                                                &bot_user));
                        } else {
                            emb = Embed(gen_embed("Success",
                                                  "set to " + words[2] +
                                                      ".\\n" + zonetime.str(),
                                                  &bot_user));
                        }
                    } catch (std::runtime_error &error) {
                        emb = Embed(err_embed(error.what(), &bot_user));
                    }
                } else if (words.size() >= 4) {
                    Server server = guildCache.at(message.serverID);
                    ServerMember mem = *server.findMember(message.author.ID);
                    Permission perms = getBasePermissions(server, mem);
                    // typo not mine
                    if (hasPremission(perms, Permission::ADMINISTRATOR) ||
                        hasPremission(perms, Permission::MANAGE_GUILD) ||
                        hasPremission(perms, Permission::KICK_MEMBERS) ||
                        hasPremission(perms, Permission::BAN_MEMBERS)) {
                        int64_t uid;
                        User user;
                        bool found = false;
                        if (std::regex_match(words[2], pong)) {
                            if (words[2].find('!') != std::string::npos) {
                                uid = std::stol(words[2].substr(3, 18), nullptr,
                                                10);
                            } else {
                                uid = std::stol(words[2].substr(2, 18), nullptr,
                                                10);
                            }
                            try {
                                user = userCache.at(Snowflake<User>(uid));
                            } catch (std::out_of_range &e) {
                                user = this->getUser(Snowflake<User>(uid));
                            }
                            found = true;
                        } else {
                            Server server = guildCache.at(message.serverID);
                            std::string name = "";
                            if (words.size() > 4) {
                                for (uint i = 2; i < words.size() - 1; i++) {
                                    name.append(words[i]);
                                    name.append(" ");
                                }
                                name.pop_back();
                            } else {
                                name = words[2];
                            }
                            for (ServerMember &x : server.members) {
                                if (x.user.username.compare(name) == 0 ||
                                    x.nick.compare(name) == 0) {
                                    user = x.user;
                                    uid = x.ID.number();
                                    found = true;
                                    break;
                                }
                            }
                        }
                        if (!found) {
                            emb = Embed(err_embed(
                                "User not found. Please make sure to type "
                                "their username/nick exactly.",
                                &bot_user));
                        } else {
                            try {
                                const date::time_zone *zone =
                                    date::locate_zone(words.back());
                                auto time = date::make_zoned(
                                    zone,
                                    date::floor<std::chrono::seconds>(
                                        std::chrono::system_clock::now()));
                                std::ostringstream zonetime;
                                zonetime << "current time: " << time;
                                sqlite3_stmt *stmt;
                                sqlite3_prepare_v2(
                                    userDB.get(), set_tz_query.c_str(),
                                    set_tz_query.length(), &stmt, nullptr);
                                sqlite3_bind_int64(stmt, 1, user.ID.number());
                                sqlite3_bind_text(stmt, 2, zone->name().c_str(),
                                                  zone->name().length(),
                                                  SQLITE_STATIC);
                                int rc = sqlite3_step(stmt);
                                if (rc != SQLITE_DONE) {
                                    std::string sql_error(
                                        sqlite3_errmsg(userDB.get()));
                                    emb = Embed(err_embed("database error: " +
                                                              sql_error,
                                                          &bot_user));
                                } else {
                                    emb = Embed(gen_embed(
                                        "Success",
                                        "set " + user.username + "#" +
                                            user.discriminator +
                                            "'s timezone to " + words.back() +
                                            ".\\n" + zonetime.str(),
                                        &bot_user));
                                }
                                sqlite3_finalize(stmt);
                            } catch (std::runtime_error &error) {
                                emb = Embed(err_embed(error.what(), &bot_user));
                            }
                        }
                    } else {
                        emb = Embed(err_embed(
                            "Only users with admin, manage guild, or kick/ban "
                            "user perms can set others' timezones.",
                            &bot_user));
                    }
                } else {
                    emb = Embed(err_embed(
                        "Wrong number of arguments; see `;tz help` for usage.",
                        &bot_user));
                }
            } else if (words[1] == "get") {
                sqlite3_stmt *stmt;
                sqlite3_prepare_v2(userDB.get(), get_tz_query.c_str(),
                                   get_tz_query.length(), &stmt, nullptr);
                int64_t uid;
                User user;
                if (words.size() == 2) {
                    sqlite3_bind_int64(stmt, 1, message.author.ID.number());
                    int rc = sqlite3_step(stmt);
                    if (rc != SQLITE_ROW) {
                        std::string sql_error(sqlite3_errmsg(userDB.get()));
                        emb = Embed(err_embed("database error: " + sql_error,
                                              &bot_user));
                    } else {
                        std::string s(reinterpret_cast<char const *>(
                            sqlite3_column_text(stmt, 0)));
                        emb = Embed(
                            gen_embed(std::string("Timezone set by ")
                                          .append(message.author.username)
                                          .append("#")
                                          .append(message.author.discriminator),
                                      s, &bot_user));
                    }
                    sqlite3_finalize(stmt);
                } else if (words.size() >= 3) {
                    bool found = false;
                    if (std::regex_match(words[2], pong)) {
                        if (words[2].find('!') != std::string::npos) {
                            uid =
                                std::stol(words[2].substr(3, 18), nullptr, 10);
                        } else {
                            uid =
                                std::stol(words[2].substr(2, 18), nullptr, 10);
                        }
                        try {
                            user = userCache.at(Snowflake<User>(uid));
                        } catch (std::out_of_range &e) {
                            user = this->getUser(Snowflake<User>(uid));
                        }
                        found = true;
                    } else {
                        Server server = guildCache.at(message.serverID);
                        std::string name = "";
                        if (words.size() > 3) {
                            for (uint i = 2; i < words.size(); i++) {
                                name.append(words[i]);
                                name.append(" ");
                            }
                            name.pop_back();
                        } else {
                            name = words[2];
                        }
                        for (ServerMember &x : server.members) {
                            if (x.user.username.compare(name) == 0 ||
                                x.nick.compare(name) == 0) {
                                user = x.user;
                                uid = x.ID.number();
                                found = true;
                                break;
                            }
                        }
                    }
                    if (!found) {
                        emb = Embed(err_embed(
                            "User not found. Please make sure to type "
                            "their username/nick exactly.",
                            &bot_user));
                    } else {
                        sqlite3_bind_int64(stmt, 1, uid);
                        int rc = sqlite3_step(stmt);
                        if (rc != SQLITE_ROW) {
                            std::string sql_error(sqlite3_errmsg(userDB.get()));
                            if (sql_error.compare("no more rows available") ==
                                0) {
                                emb = Embed(err_embed(
                                    "User has not set their timezone.",
                                    &bot_user));
                            } else {
                                emb = Embed(err_embed(
                                    "database error: " + sql_error, &bot_user));
                            }
                        } else if (rc == SQLITE_ROW) {
                            std::string s(reinterpret_cast<char const *>(
                                sqlite3_column_text(stmt, 0)));
                            emb =
                                Embed(gen_embed(std::string("Timezone set by ")
                                                    .append(user.username)
                                                    .append("#")
                                                    .append(user.discriminator),
                                                s, &bot_user));
                        }
                        sqlite3_finalize(stmt);
                    }
                }
            } else if (words[1] == "time") {
                sqlite3_stmt *stmt;
                sqlite3_prepare_v2(userDB.get(), get_tz_query.c_str(),
                                   get_tz_query.length(), &stmt, nullptr);
                int64_t uid;
                User user;
                bool found = false;
                if (words.size() == 2) {
                    found = true;
                    uid = message.author.ID.number();
                    user = message.author;
                } else if (words.size() >= 3) {
                    if (std::regex_match(words[2], pong)) {
                        if (words[2].find('!') != std::string::npos) {
                            uid =
                                std::stol(words[2].substr(3, 18), nullptr, 10);
                        } else {
                            uid =
                                std::stol(words[2].substr(2, 18), nullptr, 10);
                        }
                        try {
                            user = userCache.at(Snowflake<User>(uid));
                        } catch (std::out_of_range &e) {
                            user = this->getUser(Snowflake<User>(uid));
                        }
                        found = true;
                    } else {
                        Server server = guildCache.at(message.serverID);
                        std::string name = "";
                        if (words.size() > 3) {
                            for (uint i = 2; i < words.size(); i++) {
                                name.append(words[i]);
                                name.append(" ");
                            }
                            name.pop_back();
                        } else {
                            name = words[2];
                        }
                        for (ServerMember &x : server.members) {
                            if (x.user.username.compare(name) == 0 ||
                                x.nick.compare(name) == 0) {
                                user = x.user;
                                uid = x.ID.number();
                                found = true;
                                break;
                            }
                        }
                    }
                }
                const date::time_zone *zone;
                date::zoned_time<std::chrono::duration<long>> time;
                if (!found) {
                    try {
                        zone = date::locate_zone(words[2]);
                        time = date::make_zoned(
                            zone, date::floor<std::chrono::seconds>(
                                      std::chrono::system_clock::now()));
                        std::stringstream zonetime;
                        zonetime << time;
                        emb = Embed(gen_embed(std::string("Current time in ")
                                                  .append(zone->name())
                                                  .append(":"),
                                              zonetime.str(), &bot_user));
                    } catch (std::runtime_error &e) {
                        emb = Embed(err_embed("Timezone or user not found. See "
                                              "`;tz help` for proper usage.",
                                              &bot_user));
                    }
                } else {
                    sqlite3_stmt *stmt;
                    sqlite3_prepare_v2(userDB.get(), get_tz_query.c_str(),
                                       get_tz_query.length(), &stmt, nullptr);
                    sqlite3_bind_int64(stmt, 1, uid);
                    int rc = sqlite3_step(stmt);
                    if (rc != SQLITE_ROW) {
                        std::string sql_error(sqlite3_errmsg(userDB.get()));
                        emb = Embed(err_embed(
                            std::string("database error:").append(sql_error),
                            &bot_user));
                    } else {
                        std::string s(reinterpret_cast<char const *>(
                            sqlite3_column_text(stmt, 0)));
                        try {
                            zone = date::locate_zone(s);
                            time = date::make_zoned(
                                zone, date::floor<std::chrono::seconds>(
                                          std::chrono::system_clock::now()));
                            std::stringstream zonetime;
                            zonetime << time;
                            emb = Embed(gen_embed(
                                std::string("Current time for ")
                                    .append(user.username)
                                    .append("#")
                                    .append(user.discriminator)
                                    .append(":"),
                                std::string("(").append(s).append(") ").append(
                                    zonetime.str()),
                                &bot_user));
                        } catch (std::runtime_error &_) {
                            emb = Embed(err_embed(
                                s.append(" not found in timezone database"),
                                &bot_user));
                        }
                    }
                }
            } else {
                emb = Embed(err_embed("Command not found. Try `;tz help` for a "
                                      "list of valid commands.",
                                      &bot_user));
            }
            if (!emb.empty()) {
                sendMessage(message.channelID, "", emb);
            }
        }
    }
}
void TzBotClient::onServer(Server server) {
    guildCache.insert({server.ID, server});
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
