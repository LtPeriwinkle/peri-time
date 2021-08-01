#include "tz_client.hpp"
#include "sleepy_discord/sleepy_discord.h"
#include <regex>
#include <sqlite3.h>

static const std::regex pong = std::regex("<@!?[0-9]{18,}>");

void TzBotClient::onMessage(SleepyDiscord::Message message) {
    if (message.author.ID != botUser.ID && message.startsWith(";tz")) {
        SleepyDiscord::Embed emb(doCommand(&message));
        if (!emb.empty()) {
            sendMessage(message.channelID, "", emb);
        }
    }
}
void TzBotClient::onServer(SleepyDiscord::Server server) {
    guildCache.insert({server.ID, server});
    for (SleepyDiscord::User &user : server.members) {
        userCache.insert({user.ID, user});
    }
}
void TzBotClient::onReady(SleepyDiscord::Ready readyData) {
    sqlite3 *db;
    int rc = sqlite3_open("tz.db", &db);
    if (rc) {
        std::cerr << "Could not open tz.db file" << std::endl;
        this->quit();
    }
    userDB = sqlite3_handle{db};
    botUser = this->getCurrentUser();
}
