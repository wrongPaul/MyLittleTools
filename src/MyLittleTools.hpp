#include "rack.hpp"

using namespace rack;

// Forward-declare the Plugin, defined in Template.cpp
extern Plugin *pluginInstance;

// Forward-declare each Model, defined in each module source file
extern Model *modelMyLittleFavorites;

static const std::string lightPanelID = "LightMode";
static const std::string darkPanelID = "DarkMode";

void saveDarkAsDefault(bool darkAsDefault);
bool loadDarkAsDefault();

struct DarkDefaultItem : MenuItem {
  void onAction(const event::Action &e) override {
    saveDarkAsDefault(rightText.empty());  // implicitly toggled
  }
};