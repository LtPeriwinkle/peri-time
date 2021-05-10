#include "tz_client.hpp"
#include "sleepy_discord/sleepy_discord.h"

using namespace SleepyDiscord;

void TzBotClient::onMessage(Message message) {
  std::vector<std::string> words;
  std::string msg;
  std::stringstream ss(message.content);
  while (ss >> msg) {
    words.push_back(msg);
  }
  msg = "";
  Embed emb = Embed(Embed::Flag::INVALID_EMBED);
  if (words[0] == ";test") {
    std::string cmd = words[1];
    if (cmd == "quit" && message.author.ID == 658861212657909791) {
      this->quit();
      return;
    } else if (cmd == "something") {
      emb = Embed(R"({"title": "prototype bot"})");
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
