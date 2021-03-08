#include "modification.hpp"
#include <fstream>
#include <rapidjson/document.h>
#include <common.hpp>

namespace cppcraft
{
  Modification::Modification(std::string mname)
    : m_name(mname)
  {
    m_modpath = "mod/" + m_name;

    std::ifstream file(this->modpath() + "/mod.json");
    const std::string file_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    rapidjson::Document doc;
    doc.Parse(file_str.c_str());

    CC_ASSERT(doc.IsObject(), "Mod JSON must be valid");
    if (doc.HasMember("files"))
    {
      auto array = doc["files"].GetArray();
      for (auto& str : array) {
        m_json_files.push_back(this->modpath() + "/" + str.GetString());
      }
    }
  } // constructor

}
