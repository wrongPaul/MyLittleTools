#include <algorithm>
#include <functional>
#include <random>
#include <ui/Tooltip.hpp>
#include "MyLittleTools.hpp"
#include "plugin.hpp"
#include "window.hpp"

static const char *convertAndCombine(std::string input1, int input2) {
  char *buffer = new char[256];
  sprintf(buffer, "%s%d", input1.c_str(), input2);
  return buffer;
}

struct MyLittleTools : Module {
  enum ParamIds {
    NUM_PARAMS

  };
  enum InputIds {
    MODULE_1,
    NUM_INPUTS
  };
  enum OutputIds {
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  std::string *_plugin;
  std::string *_module;

  int _slot;
  bool _editMode;
  int panelTheme;

  MyLittleTools() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    _plugin = new std::string[8];
    _module = new std::string[8];
    _editMode = false;
    panelTheme = (loadDarkAsDefault() ? 1 : 0);
  }

  void RaiseModel(std::string plugin, std::string module) {
    Model *model = rack::plugin::getPlugin(plugin)->getModel(module);
    if (model) {
      ModuleWidget *moduleWidget = model->createModuleWidget();
      if (!moduleWidget) {
        return;
      }
      APP->scene->rack->addModuleAtMouse(moduleWidget);
      history::ModuleAdd *h = new history::ModuleAdd;
      h->name = "create module";
      h->setModule(moduleWidget);
      APP->history->push(h);
    }
  }

  void setEditMode(bool value) {
    _editMode = value;
  }

  bool getEditMode() {
    return _editMode;
  }

  std::string getSavedModule(int index) {
    return _module[index];
  }

  std::string getSavedPlugin(int index) {
    return _plugin[index];
  }

  void setSavedModule(int index, std::string module) {
    _module[index] = module;
  }

  void setSlot(int slot) {
    _slot = slot;
  }

  void setSavedPlugin(int index, std::string plugin) {
    _plugin[index] = plugin;
  }

  void onReset() override {
    for (int i = 0; i < 8; i++) {
      setSavedPlugin(i, "");
      setSavedModule(i, "");
    }
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();

    for (int i = 0; i < 8; i++) {
      if (_plugin[i] != "" && _module[i] != "") {
        json_object_set_new(rootJ, convertAndCombine("plugin", i), json_string(_plugin[i].c_str()));
        json_object_set_new(rootJ, convertAndCombine("module", i), json_string(_module[i].c_str()));
      }
    }

    json_object_set_new(rootJ, "panelTheme", json_integer(panelTheme));

    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    //ModuleWidget::fromJson(rootJ);

    for (int i = 0; i < 8; i++) {
      json_t *plug = json_object_get(rootJ, convertAndCombine("plugin", i));
      json_t *mod = json_object_get(rootJ, convertAndCombine("module", i));

      if (plug)
        _plugin[i] = json_string_value(plug);

      if (mod)
        _module[i] = json_string_value(mod);
    }
    json_t *panelThemeJ = json_object_get(rootJ, "panelTheme");
    if (panelThemeJ)
      panelTheme = json_integer_value(panelThemeJ);
  }

  void process(const ProcessArgs &args) override {
  }
};

struct ModuleMenuItem : ui::MenuItem {
  MyLittleTools *module;
  int slot;
  std::string pluginName;
  std::string moduleName;

  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  void onAction(const event::Action &e) override {
    module->setSavedModule(slot, moduleName);
    module->setSavedPlugin(slot, pluginName);
  }
};

struct heartButton : SvgButton {
  MyLittleTools *module;

  std::shared_ptr<Svg> svg1;
  std::shared_ptr<Svg> svg2;

  ui::Label *labelEditMode;

  heartButton() {
    svg1 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/heart1.svg"));
    svg2 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/heart2.svg"));

    addFrame(svg1);

    labelEditMode = new ui::Label;
    // brandLabel->fontSize = 16;
    labelEditMode->box.pos.x = 22;
    labelEditMode->box.pos.y = 0;
    labelEditMode->color = nvgRGB(0x30, 0x30, 0x30);
    //labelEditMode->text = "edit mode";
    addChild(labelEditMode);
    viewEditMode(false);
  }

  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  void viewEditMode(bool mode) {
    if (mode)
      labelEditMode->text = "edit mode";
    else
      labelEditMode->text = "<- edit";
  }

  virtual void onAction(const event::Action &e) override {
    if (module->getEditMode()) {
      module->setEditMode(false);
      frames[0] = svg1;
      viewEditMode(false);
    } else {
      module->setEditMode(true);
      frames[0] = svg2;
      viewEditMode(true);
    }
  }
};

struct slotButton : SvgButton {
  MyLittleTools *module;
  int buttonid;
  ui::Label *labelName;

  slotButton() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/dark/sb0_b.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/dark/sb1.svg_b")));
    labelName = new ui::Label;
    labelName->box.pos.x = 5;
    labelName->box.pos.y = 2;
    labelName->color = nvgRGB(0x10, 0x10, 0x10);

    addChild(labelName);
  }

  void step() override {
    if (module) {
      setLabelName();
    }
    Widget::step();
  };

  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  void setLabelName() {
    if (module) {
      std::string str = module->getSavedModule(buttonid);
      unsigned sz = str.size();
      if (sz > 15)
        str.resize(sz + 3, '.');

      if (labelName->text != str)
        labelName->text = str;
    }
  }

  virtual void onAction(const event::Action &e) override {
    if (module->getEditMode())
      makeContextMenuOnButton(buttonid);
    else {
      if (module->getSavedPlugin(buttonid) != "") {
        labelName->text = module->getSavedModule(buttonid);
        module->RaiseModel(module->getSavedPlugin(buttonid), module->getSavedModule(buttonid));
      }
    }
  }

  void makeContextMenuOnButton(int slot) {
    ui::Menu *menu = createMenu();
    menu->addChild(createMenuLabel("currently loaded Rack modules"));

    for (widget::Widget *w : APP->scene->rack->moduleContainer->children) {
      ModuleWidget *mw = dynamic_cast<ModuleWidget *>(w);

      json_t *moduleJ = mw->toJson();
      json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
      json_t *modelSlugJ = json_object_get(moduleJ, "model");

      ModuleMenuItem *item1 = new ModuleMenuItem;
      item1->slot = slot;
      item1->setModule(module);
      item1->text = json_string_value(pluginSlugJ);
      item1->rightText = json_string_value(modelSlugJ);
      item1->pluginName = json_string_value(pluginSlugJ);
      item1->moduleName = json_string_value(modelSlugJ);
      menu->addChild(item1);
    }
  }
};

struct MyLittleFavoritesWidget : ModuleWidget {
  TextField *textField;

  // Panel theme stuff -----------------------------------------
  SvgPanel *darkPanel;

  // panel context menu --------
  struct PanelThemeItem : MenuItem {
    MyLittleTools *module;
    int theme;
    void onAction(const event::Action &e) override {
      module->panelTheme = theme;
    }
    void step() override {
      rightText = (module->panelTheme == theme) ? "✔" : "";
    }
  };
  void appendContextMenu(Menu *menu) override {
    MenuLabel *spacerLabel = new MenuLabel();
    menu->addChild(spacerLabel);

    MyLittleTools *module = dynamic_cast<MyLittleTools *>(this->module);
    assert(module);

    MenuLabel *themeLabel = new MenuLabel();
    themeLabel->text = "Panel Theme";
    menu->addChild(themeLabel);

    PanelThemeItem *lightItem = new PanelThemeItem();
    lightItem->text = lightPanelID;  //.hpp
    lightItem->module = module;
    lightItem->theme = 0;
    menu->addChild(lightItem);

    PanelThemeItem *darkItem = new PanelThemeItem();
    darkItem->text = darkPanelID;  //.hpp
    darkItem->module = module;
    darkItem->theme = 1;
    menu->addChild(darkItem);

    menu->addChild(createMenuItem<DarkDefaultItem>("Dark as default", CHECKMARK(loadDarkAsDefault())));
  };

  MyLittleFavoritesWidget(MyLittleTools *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MyLittleFavorites.svg")));

    if (module) {
      darkPanel = new SvgPanel();
      darkPanel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/dark/MyLittleFavorites_b.svg")));

      darkPanel->visible = false;
      addChild(darkPanel);
    }

    textField = createWidget<LedDisplayTextField>(Vec(9, 58));
    textField->box.size = Vec(131, 29);
    addChild(textField);

    heartButton *hb = createWidget<heartButton>(Vec(38, 24));
    hb->setModule(module);
    addChild(hb);

    int ystart = 105;
    int yjump = 32;

    //*sb = new slotButton[8];

    for (int i = 0; i < 8; i++) {
      slotButton *sb;
      sb = createWidget<slotButton>(Vec(9, ystart));
      sb->setModule(module);
      sb->buttonid = i;
      sb->setLabelName();
      addChild(sb);
      ystart += yjump;
    }
  }

  json_t *toJson() override {
    json_t *rootJ = ModuleWidget::toJson();

    // text
    json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

    return rootJ;
  }

  void fromJson(json_t *rootJ) override {
    ModuleWidget::fromJson(rootJ);

    // text
    json_t *textJ = json_object_get(rootJ, "text");
    if (textJ)
      textField->text = json_string_value(textJ);
  }
  void step() override {
    if (module) {
      panel->visible = ((((MyLittleTools *)module)->panelTheme) == 0);
      darkPanel->visible = ((((MyLittleTools *)module)->panelTheme) == 1);
    }
    Widget::step();
  };
};

Model *modelMyLittleFavorites = createModel<MyLittleTools, MyLittleFavoritesWidget>("MyLittleFavorites");
