#include "embed_templates.hpp"
#include "sleepy_discord/sleepy_discord.h"
#include "tz_client.hpp"
#include <chrono>
#include <exception>
#include <regex>
#include <sqlite3.h>
#include <sstream>
#include <string>
#include <vector>

static const std::regex pong = std::regex("<@!?[0-9]{18,}>");

std::string TzBotClient::doCommand(SleepyDiscord::Message *message) {
    std::vector<std::string> words;
    std::string msg;
    std::stringstream ss(message->content);
    while (ss >> msg) {
        words.push_back(msg);
    }
    if (words.size() >= 2) {
        if (words[1] == "help") {
            return help_embed(&botUser);
        } else if (words[1] == "set") {
            if (words.size() >= 3) {
                return tzSet(words, message);
            } else {
                return err_embed("Wrong number of arguments; see `;tz help` "
                                 "for command usage.",
                                 &botUser);
            }
        } else if (words[1] == "get") {
            return tzGet(words, message);
        } else if (words[1] == "list") {
            return gen_embed(
                "Timezone List",
                "All available timezones and their locations can "
                "be found at https://kevinnovak.github.io/Time-Zone-Picker",
                &botUser);
        } else if (words[1] == "time") {
            return tzTime(words, message);
        } else {
            return err_embed(
                "Not a valid command. See `;tz help` for a list of commands.",
                &botUser);
        }
    } else {
        return err_embed(
            "Not a valid command. See `;tz help` for a list of commands.",
            &botUser);
    }
}

std::string TzBotClient::tzGet(const std::vector<std::string> words,
                               SleepyDiscord::Message *message) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(userDB.get(), get_tz_query.c_str(),
                       get_tz_query.length(), &stmt, nullptr);
    int64_t uid;
    SleepyDiscord::User user;
    if (words.size() == 2) {
        sqlite3_bind_int64(stmt, 1, message->author.ID.number());
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            std::string sql_error(sqlite3_errmsg(userDB.get()));
            sqlite3_finalize(stmt);
            if (sql_error == "no more rows available") {
                return err_embed("You have not set their timezone.", &botUser);
            } else {
                return err_embed("database error: " + sql_error, &botUser);
            }
        } else {
            std::string s(
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
            sqlite3_finalize(stmt);
            return gen_embed(std::string("Timezone set by ")
                                 .append(message->author.username)
                                 .append("#")
                                 .append(message->author.discriminator),
                             s, &botUser);
        }
    } else if (words.size() >= 3) {
        bool found = false;
        if (std::regex_match(words[2], pong)) {
            if (words[2].find('!') != std::string::npos) {
                uid = std::stol(words[2].substr(3, 18), nullptr, 10);
            } else {
                uid = std::stol(words[2].substr(2, 18), nullptr, 10);
            }
            try {
                user = userCache.at(
                    SleepyDiscord::Snowflake<SleepyDiscord::User>(uid));
            } catch (std::out_of_range &e) {
                user = this->getUser(
                    SleepyDiscord::Snowflake<SleepyDiscord::User>(uid));
            }
            found = true;
        } else {
            SleepyDiscord::Server server = guildCache.at(message->serverID);
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
            for (SleepyDiscord::ServerMember &x : server.members) {
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
            return err_embed("User not found. Please make sure to type "
                             "their username/nick exactly.",
                             &botUser);
        } else {
            sqlite3_bind_int64(stmt, 1, uid);
            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_ROW) {
                std::string sql_error(sqlite3_errmsg(userDB.get()));
                sqlite3_finalize(stmt);
                if (sql_error == "no more rows available") {
                    return err_embed("User has not set their timezone.",
                                     &botUser);
                } else {
                    return err_embed("database error: " + sql_error, &botUser);
                }
            } else {
                std::string s(reinterpret_cast<const char *>(
                    sqlite3_column_text(stmt, 0)));
                sqlite3_finalize(stmt);
                return gen_embed(std::string("Timezone set by ")
                                     .append(user.username)
                                     .append("#")
                                     .append(user.discriminator),
                                 s, &botUser);
            }
        }
    } else {
        throw std::runtime_error("How did we get here?");
    }
}

std::string TzBotClient::tzSet(const std::vector<std::string> words,
                               SleepyDiscord::Message *message) {
    if (words.size() == 3) {
        try {
            std::string zonetime = getZoneTime(words[2]);
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(userDB.get(), set_tz_query.c_str(),
                               set_tz_query.length(), &stmt, nullptr);
            sqlite3_bind_int64(stmt, 1, message->author.ID.number());
            sqlite3_bind_text(stmt, 2, words[2].c_str(), words[2].length(),
                              SQLITE_STATIC);
            int rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                std::string sql_error(sqlite3_errmsg(userDB.get()));
                sqlite3_finalize(stmt);
                return err_embed(
                    std::string("database error: ").append(sql_error),
                    &botUser);
            } else {
                sqlite3_finalize(stmt);
                return gen_embed("Success",
                                 std::string("set to ")
                                     .append(words[2])
                                     .append(".\\n")
                                     .append(zonetime),
                                 &botUser);
            }
        } catch (std::runtime_error &error) {
            return err_embed(words[2] + " not found in timezone database. (try capitalizing place names)",
                             &botUser);
        }
    } else if (words.size() >= 4) {
        SleepyDiscord::Server server = guildCache.at(message->serverID);
        SleepyDiscord::ServerMember mem =
            *server.findMember(message->author.ID);
        SleepyDiscord::Permission perms = getBasePermissions(server, mem);
        // typo not mine
        if (hasPremission(perms, SleepyDiscord::Permission::ADMINISTRATOR) ||
            hasPremission(perms, SleepyDiscord::Permission::MANAGE_GUILD) ||
            hasPremission(perms, SleepyDiscord::Permission::KICK_MEMBERS) ||
            hasPremission(perms, SleepyDiscord::Permission::BAN_MEMBERS)) {
            int64_t uid;
            SleepyDiscord::User user;
            bool found = false;
            if (std::regex_match(words[2], pong)) {
                if (words[2].find('!') != std::string::npos) {
                    uid = std::stol(words[2].substr(3, 18), nullptr, 10);
                } else {
                    uid = std::stol(words[2].substr(2, 18), nullptr, 10);
                }
                try {
                    user = userCache.at(
                        SleepyDiscord::Snowflake<SleepyDiscord::User>(uid));
                } catch (std::out_of_range &e) {
                    user = this->getUser(
                        SleepyDiscord::Snowflake<SleepyDiscord::User>(uid));
                }
                found = true;
            } else {
                SleepyDiscord::Server server = guildCache.at(message->serverID);
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
                for (SleepyDiscord::ServerMember &x : server.members) {
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
                return err_embed("User not found. Please make sure to type "
                                 "their username/nick exactly.",
                                 &botUser);
            } else {
                try {
                    std::string zonetime = getZoneTime(words.back());
                    sqlite3_stmt *stmt;
                    sqlite3_prepare_v2(userDB.get(), set_tz_query.c_str(),
                                       set_tz_query.length(), &stmt, nullptr);
                    sqlite3_bind_int64(stmt, 1, user.ID.number());
                    sqlite3_bind_text(stmt, 2, words.back().c_str(),
                                      words.back().length(), SQLITE_STATIC);
                    int rc = sqlite3_step(stmt);
                    if (rc != SQLITE_DONE) {
                        std::string sql_error(sqlite3_errmsg(userDB.get()));
                        sqlite3_finalize(stmt);
                        return err_embed(
                            std::string("database error: ").append(sql_error),
                            &botUser);
                    } else {
                        sqlite3_finalize(stmt);
                        return gen_embed("Success",
                                         std::string("set ")
                                             .append(user.username)
                                             .append("#")
                                             .append(user.discriminator)
                                             .append("'s timezone to ")
                                             .append(words.back())
                                             .append(".\\n")
                                             .append("Current time: ")
                                             .append(zonetime),
                                         &botUser);
                    }
                } catch (std::runtime_error &error) {
                    return err_embed(error.what(), &botUser);
                }
            }
        } else {
            return err_embed("Only users with admin, manage guild, or kick/ban "
                             "user perms can set others' timezones.",
                             &botUser);
        }
    } else {
        throw std::runtime_error("How did we get here?");
    }
}

std::string TzBotClient::tzTime(const std::vector<std::string> words,
                                SleepyDiscord::Message *message) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(userDB.get(), get_tz_query.c_str(),
                       get_tz_query.length(), &stmt, nullptr);
    int64_t uid;
    SleepyDiscord::User user;
    bool found = false;
    if (words.size() == 2) {
        found = true;
        uid = message->author.ID.number();
        user = message->author;
    } else if (words.size() >= 3) {
        if (std::regex_match(words[2], pong)) {
            if (words[2].find('!') != std::string::npos) {
                uid = std::stol(words[2].substr(3, 18), nullptr, 10);
            } else {
                uid = std::stol(words[2].substr(2, 18), nullptr, 10);
            }
            try {
                user = userCache.at(
                    SleepyDiscord::Snowflake<SleepyDiscord::User>(uid));
            } catch (std::out_of_range &e) {
                user = this->getUser(
                    SleepyDiscord::Snowflake<SleepyDiscord::User>(uid));
            }
            found = true;
        } else {
            SleepyDiscord::Server server = guildCache.at(message->serverID);
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
            for (SleepyDiscord::ServerMember &x : server.members) {
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
    if (!found) {
        try {
            return gen_embed(
                std::string("Current time in ").append(words[2]).append(":"),
                getZoneTime(words[2]), &botUser);
        } catch (std::runtime_error &e) {
            return err_embed("Timezone or user not found (did you type their "
                             "username/nickname exactly?)\\n See "
                             "`;tz help` for proper usage.",
                             &botUser);
        }
    } else {
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(userDB.get(), get_tz_query.c_str(),
                           get_tz_query.length(), &stmt, nullptr);
        sqlite3_bind_int64(stmt, 1, uid);
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            std::string sql_error(sqlite3_errmsg(userDB.get()));
            return err_embed(std::string("database error: ").append(sql_error),
                             &botUser);
        } else {
            std::string s(
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
            try {
                return gen_embed(std::string("Current time for ")
                                     .append(user.username)
                                     .append("#")
                                     .append(user.discriminator)
                                     .append(":"),
                                 std::string("(").append(s).append(") ").append(
                                     getZoneTime(s)),
                                 &botUser);
            } catch (std::runtime_error &_) {
                return err_embed(s.append(" not found in timezone database. (try capitalizing place names.)"),
                                 &botUser);
            }
        }
    }
}

std::string TzBotClient::getZoneTime(const std::string name) {
    const date::time_zone *zone = date::locate_zone(name);
    auto time = date::make_zoned(zone, date::floor<std::chrono::seconds>(
                                           std::chrono::system_clock::now()));
    std::stringstream zonetime;
    zonetime << time;
    return zonetime.str();
}
