#include "embed_templates.hpp"
#include <string>
#include <sstream>

std::string gen_embed(std::string title, std::string description, int color) {
  std::stringstream ss;
  ss << R"({"title": ")" << title << R"(", "description": ")" << description << R"(", "color": )" << color << "}";
  return ss.str();
}
