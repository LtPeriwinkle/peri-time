#define SLEEPY_DEFAULT_REQUEST_MODE SleepyDiscord::Async
#include "sleepy_discord/sleepy_discord.h"
#include <iostream>
#include <fstream>

using namespace SleepyDiscord;

class TzBotClient: public DiscordClient {
public:
    using DiscordClient::DiscordClient;
    void onMessage(Message message) override {
        if (message.startsWith(";cpptest")) {
            sendMessage(message.channelID, "this is working");
        }
    }
    void onServer(Server server) override {
        for (Channel& channel : server.channels) {
            channelCache.insert({channel.ID, channel});
        }
    }
private:
    std::unordered_map<
        Snowflake<Channel>::RawType,
        Channel
    > channelCache;
};

int main() {
    std::ifstream tokFile(".bot-token");
    std::string botToken;
    getline(tokFile, botToken);
    TzBotClient client(botToken, USER_CONTROLED_THREADS);
    client.run();
}
