#include "MyLittleTools.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;

  // Add all Models defined throughout the pluginInstance
  p->addModel(modelMyLittleFavorites);

  // Any other pluginInstance initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

void saveDarkAsDefault(bool darkAsDefault) {
  json_t *settingsJ = json_object();
  json_object_set_new(settingsJ, "darkAsDefault", json_boolean(darkAsDefault));
  std::string settingsFilename = asset::user("MyLittleTools.json");
  FILE *file = fopen(settingsFilename.c_str(), "w");
  if (file) {
    json_dumpf(settingsJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
    fclose(file);
  }
  json_decref(settingsJ);
}

bool loadDarkAsDefault() {
  bool ret = false;
  std::string settingsFilename = asset::user("MyLittleTools.json");
  FILE *file = fopen(settingsFilename.c_str(), "r");
  if (!file) {
    saveDarkAsDefault(false);
    return ret;
  }
  json_error_t error;
  json_t *settingsJ = json_loadf(file, 0, &error);
  if (!settingsJ) {
    // invalid setting json file
    fclose(file);
    saveDarkAsDefault(false);
    return ret;
  }
  json_t *darkAsDefaultJ = json_object_get(settingsJ, "darkAsDefault");
  if (darkAsDefaultJ)
    ret = json_boolean_value(darkAsDefaultJ);

  fclose(file);
  json_decref(settingsJ);
  return ret;
}
