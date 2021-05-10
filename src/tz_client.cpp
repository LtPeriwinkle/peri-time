#include "tz_client.hpp"
#include "sleepy_discord/sleepy_discord.h"

using namespace SleepyDiscord;

void TzBotClient::onMessage(Message message) {
  if (message.startsWith(";test")) {
    std::vector<std::string> words;
    std::string msg;
    std::stringstream ss(message.content);
    while (ss >> msg) {
      words.push_back(msg);
    }
    msg = "";
    Embed emb = Embed(Embed::Flag::INVALID_EMBED);
    if (words[1] == "quit" && (message.author.ID.number() == 658861212657909791)) {
      sendMessage(message.channelID, "bye");
      this->quit();
      return;
    } else if (words[1] == "something") {
      emb = Embed(R"({"title": "prototype tzbot"})");
    } else {
      msg = "unknown";
    } 
    sendMessage(message.channelID, msg, emb);
  }
}
void TzBotClient::onServer(Server server) {
  for (User &user : server.members) {
    userCache.insert({user.ID, user});
  }
}
