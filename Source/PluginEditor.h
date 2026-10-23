#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include "PluginProcessor.h"

namespace ui {
// final design palette: black carbon, brushed red metal, neon red readouts
const juce::Colour kBg    (0xff050202);
const juce::Colour kPanel (0xff150808);
const juce::Colour kGold  (0xffff5a4e);   // neon red (main accent)
const juce::Colour kGold2 (0xffa8201a);   // deep red (borders)
const juce::Colour kText  (0xfff2dcd6);
const juce::Colour kDim   (0xffa58078);
const juce::Colour kCyan  (0xffff8f80);   // soft neon (readouts)
const juce::Colour kMagenta(0xffff2a1a);  // hot red (off-pitch)
}

class DiscoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DiscoLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h, float pos, float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

// look used on the artwork screen: the artwork provides the knob / frame graphics, code draws only what changes
class ArtLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ArtLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h, float pos, float startAngle, float endAngle, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int w, int h, float sliderPos, float minPos, float maxPos, juce::Slider::SliderStyle, juce::Slider&) override;
    int  getSliderThumbRadius(juce::Slider&) override;
    void drawComboBox(juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override {}
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override {}
};

// the always-open "DOUBLER MODE" list from the design
class ModeList : public juce::Component
{
public:
    std::function<void(int)> onSelect;
    void setSelected(int i) { if (i != selected) { selected = i; repaint(); } }
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
private:
    int selected = 1;
};

// rotary knob + caption, bound to one parameter
class KnobComp : public juce::Component
{
public:
    KnobComp(juce::AudioProcessorValueTreeState& s, int paramIndex, const juce::String& caption = {});
    void resized() override;
private:
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;   // keep last
};

class ToggleComp : public juce::Component
{
public:
    ToggleComp(juce::AudioProcessorValueTreeState& s, int paramIndex, const juce::String& caption = {});
    void resized() override { button.setBounds(getLocalBounds()); }
private:
    juce::ToggleButton button;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> att;
};

class ComboComp : public juce::Component
{
public:
    ComboComp(juce::AudioProcessorValueTreeState& s, int paramIndex, const juce::String& caption = {});
    void resized() override;
private:
    juce::Label label;
    juce::ComboBox box;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> att;
};

// a titled section of the main page
class Panel : public juce::Component
{
public:
    Panel(juce::AudioProcessorValueTreeState& s, const juce::String& title) : state(s), titleText(title) {}
    void addToggle(int p, const juce::String& cap = {}) { strip.push_back(std::make_unique<ToggleComp>(state, p, cap)); addAndMakeVisible(*strip.back()); }
    void addCombo(int p, const juce::String& cap = {})  { strip.push_back(std::make_unique<ComboComp>(state, p, cap)); addAndMakeVisible(*strip.back()); stripWide.push_back(strip.size() - 1); }
    void addKnob(int p, const juce::String& cap = {})   { knobs.push_back(std::make_unique<KnobComp>(state, p, cap)); addAndMakeVisible(*knobs.back()); }
    juce::Label extra;                      // optional readout (e.g. gain reduction)
    void enableExtra() { extra.setJustificationType(juce::Justification::centredRight); extra.setColour(juce::Label::textColourId, ui::kCyan); extra.setFont(juce::Font(12.0f)); addAndMakeVisible(extra); }
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    juce::AudioProcessorValueTreeState& state;
    juce::String titleText;
    std::vector<std::unique_ptr<juce::Component>> strip;
    std::vector<size_t> stripWide;
    std::vector<std::unique_ptr<KnobComp>> knobs;
};

// "OPEN MAP": every single parameter, generated from the parameter table
class MapContent : public juce::Component
{
public:
    explicit MapContent(juce::AudioProcessorValueTreeState& s);
    void resized() override;
    int requiredHeight() const { return totalH; }
private:
    struct Row { int header = 0; juce::String text; std::unique_ptr<juce::Label> label; std::unique_ptr<juce::Component> control; std::unique_ptr<juce::Slider> slider; std::unique_ptr<juce::ToggleButton> toggle; std::unique_ptr<juce::ComboBox> combo;
                 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sa; std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> ba; std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ca; };
    std::vector<std::unique_ptr<Row>> rows;
    int totalH = 0;
};

class DiscoSauceEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit DiscoSauceEditor(DiscoSauceProcessor&);
    ~DiscoSauceEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    enum View { ViewArt = 0, ViewPanels = 1, ViewMap = 2 };
    struct ArtKnob { juce::Slider s; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a; int param = 0; float cx = 0, cy = 0, size = 0; };

    void timerCallback() override;
    void setView(int v);
    void presetChanged();
    void layoutArt();
    void layoutPanels();
    void paintTuner(juce::Graphics&);

    DiscoSauceProcessor& proc;
    DiscoLookAndFeel laf;
    ArtLookAndFeel artLaf;
    int view = ViewArt;

    // artwork screen
    std::vector<std::unique_ptr<ArtKnob>> artKnobs;
    juce::Slider dryWet;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAtt;
    juce::ComboBox keyBox, scaleBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAtt, scaleAtt;
    juce::ToggleButton tuneToggle { "TUNE ON" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> tuneAtt;
    ModeList modeList;

    // detailed panels screen
    juce::Component mainPage;
    std::vector<std::unique_ptr<Panel>> panels;
    Panel* compPanel = nullptr;

    // open mixer screen (every parameter)
    juce::Viewport mapViewport;
    std::unique_ptr<MapContent> mapContent;

    // shared
    juce::ComboBox presetBox;
    juce::Slider presetAmount;
    juce::TextButton panelsButton { "PRO PANELS" }, mapButton { "OPEN MIXER" }, saveButton { "SAVE" }, loadButton { "LOAD" };
    std::unique_ptr<juce::FileChooser> chooser;

    juce::Image artwork;
    juce::Rectangle<float> tunerBox;
    juce::String tunerIn, tunerOut;
    bool tunerVoiced = false;
    float tunerCents = 0.0f;
    float kScale = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiscoSauceEditor)
};
