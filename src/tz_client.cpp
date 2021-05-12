#include "tz_client.hpp"
#include "embed_templates.hpp"
#include "sleepy_discord/sleepy_discord.h"
#include <chrono>
#include <date/tz.h>
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
      if (words[1] == "quit" &&
          (message.author.ID.number() == 658861212657909791)) {
        sendMessage(message.channelID, "bye");
        this->quit();
        return;
      } else if (words[1] == "help") {
        emb = Embed(gen_embed("tzbot help", "this will be something eventually",
                              &userCache.at(this->getID())));
      } else if (words[1] == "set") {
        if (words.size() == 3) {
          /* make sqlite call here lmaooo */
          try {
            const date::time_zone *zone =
                date::get_tzdb().locate_zone(words[2]);
            auto time =
                date::make_zoned(zone, date::floor<std::chrono::seconds>(
                                           std::chrono::system_clock::now()));
            std::stringstream zonetime;
            zonetime << "current time: " << time;
            emb = Embed(gen_embed("Success",
                                  "set to " + words[2] + ", " + zonetime.str(),
                                  &userCache.at(this->getID())));
          } catch (std::runtime_error &error) {
            emb = Embed(err_embed(error.what(), &userCache.at(this->getID())));
          }
        } else {
          emb = Embed(
              err_embed("Not enough arguments!", &userCache.at(this->getID())));
        }
      } else {
        emb =
            Embed(err_embed("Command not found", &userCache.at(this->getID())));
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
