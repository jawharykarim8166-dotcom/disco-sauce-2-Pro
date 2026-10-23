#include "PluginEditor.h"
#include <algorithm>
#if DS_HAS_ARTWORK
 #include <BinaryData.h>
#endif

using namespace dsp2;

// ===================================================================== look and feel
DiscoLookAndFeel::DiscoLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, ui::kText);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId, ui::kText);
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a0d0d));
    setColour(juce::ComboBox::textColourId, ui::kText);
    setColour(juce::ComboBox::outlineColourId, ui::kGold2);
    setColour(juce::ComboBox::arrowColourId, ui::kGold);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff170808));
    setColour(juce::PopupMenu::textColourId, ui::kText);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, ui::kGold2);
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a0d0d));
    setColour(juce::TextButton::textColourOffId, ui::kGold);
    setColour(juce::Slider::trackColourId, ui::kGold);
    setColour(juce::Slider::backgroundColourId, juce::Colour(0xff3a1814));
    setColour(juce::Slider::thumbColourId, ui::kText);
}

void DiscoLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    const auto b = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(4.0f);
    const float d = std::min(b.getWidth(), b.getHeight()), r = d * 0.5f;
    const auto c = b.getCentre();
    const float ang = a0 + pos * (a1 - a0);
    g.setColour(juce::Colour(0xff2a0d0d));
    g.fillEllipse(c.x - r, c.y - r, d, d);
    juce::Path track; track.addCentredArc(c.x, c.y, r - 2.0f, r - 2.0f, 0.0f, a0, a1, true);
    g.setColour(juce::Colour(0xff4a1c18)); g.strokePath(track, juce::PathStrokeType(3.0f));
    juce::Path val; val.addCentredArc(c.x, c.y, r - 2.0f, r - 2.0f, 0.0f, a0, ang, true);
    g.setGradientFill(juce::ColourGradient(ui::kGold, c.x, c.y - r, ui::kGold2, c.x, c.y + r, false));
    g.strokePath(val, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    juce::Path tick; tick.addRoundedRectangle(-1.5f, -r + 6.0f, 3.0f, r * 0.42f, 1.5f);
    g.setColour(ui::kText); g.fillPath(tick, juce::AffineTransform::rotation(ang).translated(c.x, c.y));
    juce::ignoreUnused(s);
}

void DiscoLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    const auto r = b.getLocalBounds().toFloat().reduced(1.0f);
    const bool on = b.getToggleState();
    g.setColour(on ? ui::kGold : juce::Colour(0xff2a1210));
    g.fillRoundedRectangle(r, 5.0f);
    g.setColour(on ? ui::kGold2 : juce::Colour(0xff7a3a2c));
    g.drawRoundedRectangle(r, 5.0f, 1.2f);
    g.setColour(on ? juce::Colour(0xff150500) : ui::kDim);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText(b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

ArtLookAndFeel::ArtLookAndFeel()
{
    setColour(juce::ComboBox::textColourId, juce::Colour(0xffffa093));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff160808));
    setColour(juce::PopupMenu::textColourId, ui::kText);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, ui::kGold2);
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour(juce::Label::textColourId, ui::kText);
}

void ArtLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&)
{
    const auto b = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h);
    const auto c = b.getCentre();
    const float r = std::min(b.getWidth(), b.getHeight()) * 0.5f;
    const float ang = a0 + pos * (a1 - a0);

    juce::Path groove; groove.addCentredArc(c.x, c.y, r * 0.93f, r * 0.93f, 0.0f, a0, a1, true);
    g.setColour(juce::Colour(0xff180606));                                        // recessed track (also hides the static arc in the artwork)
    g.strokePath(groove, juce::PathStrokeType(r * 0.115f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(juce::Colour(0xff3a1210));
    g.strokePath(groove, juce::PathStrokeType(r * 0.115f + 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded), juce::AffineTransform());
    g.setColour(juce::Colour(0xff180606));
    g.strokePath(groove, juce::PathStrokeType(r * 0.10f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path arc; arc.addCentredArc(c.x, c.y, r * 0.93f, r * 0.93f, 0.0f, a0, ang, true);
    g.setColour(ui::kGold.withAlpha(0.28f));
    g.strokePath(arc, juce::PathStrokeType(r * 0.16f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));   // neon glow
    g.setColour(ui::kGold);
    g.strokePath(arc, juce::PathStrokeType(r * 0.055f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(juce::Colour(0xffffd0c8));
    g.strokePath(arc, juce::PathStrokeType(r * 0.02f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto t = juce::AffineTransform::rotation(ang).translated(c.x, c.y);
    juce::Path tick; tick.addRoundedRectangle(-r * 0.034f, -r * 0.62f, r * 0.068f, r * 0.20f, r * 0.03f);
    g.setColour(juce::Colours::black.withAlpha(0.7f)); g.fillPath(tick, t);
    juce::Path tick2; tick2.addRoundedRectangle(-r * 0.02f, -r * 0.61f, r * 0.04f, r * 0.18f, r * 0.02f);
    g.setColour(ui::kCyan); g.fillPath(tick2, t);
}

int ArtLookAndFeel::getSliderThumbRadius(juce::Slider& s)
{
    return s.isHorizontal() ? (int) ((float) s.getHeight() * 0.58f) : (int) ((float) s.getWidth() * 0.26f);
}

void ArtLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float sliderPos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    auto metal = [&](juce::Rectangle<float> th, float corner, bool vertical)
    {
        g.setColour(juce::Colours::black.withAlpha(0.5f)); g.fillRoundedRectangle(th.translated(1.5f, 2.5f), corner);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xfff0e8e6), 0, th.getY(), juce::Colour(0xff6c6462), 0, th.getBottom(), false));
        g.fillRoundedRectangle(th, corner);
        g.setColour(juce::Colour(0xff201414)); g.drawRoundedRectangle(th, corner, 1.4f);
        const float cy = th.getCentreY(), cx = th.getCentreX();
        g.setColour(ui::kGold.withAlpha(0.35f));
        if (vertical) g.fillRect(th.getX() + th.getWidth() * 0.1f, cy - 3.0f, th.getWidth() * 0.8f, 6.0f);
        else          g.fillRect(cx - 3.0f, th.getY() + th.getHeight() * 0.1f, 6.0f, th.getHeight() * 0.8f);
        g.setColour(ui::kGold);
        if (vertical) g.fillRect(th.getX() + th.getWidth() * 0.12f, cy - 1.0f, th.getWidth() * 0.76f, 2.0f);
        else          g.fillRect(cx - 1.0f, th.getY() + th.getHeight() * 0.12f, 2.0f, th.getHeight() * 0.76f);
    };
    if (s.isHorizontal())
    {
        const auto slot = juce::Rectangle<float>((float) x, (float) y + (float) h * 0.22f, (float) w, (float) h * 0.56f);
        g.setColour(juce::Colour(0xff150707)); g.fillRoundedRectangle(slot, slot.getHeight() * 0.4f);
        g.setColour(juce::Colour(0xff3c1612)); g.drawRoundedRectangle(slot, slot.getHeight() * 0.4f, 1.2f);
        const int dots = 34; const float x0 = slot.getX() + slot.getHeight() * 0.7f, x1 = slot.getRight() - slot.getHeight() * 0.7f;
        const float dr = slot.getHeight() * 0.2f;
        for (int i = 0; i < dots; ++i)                                      // LED-dot fill like the design
        {
            const float dx = x0 + (x1 - x0) * (float) i / (float) (dots - 1);
            const bool lit = dx <= sliderPos;
            if (lit) { g.setColour(ui::kGold.withAlpha(0.3f)); g.fillEllipse(dx - dr * 1.8f, slot.getCentreY() - dr * 1.8f, dr * 3.6f, dr * 3.6f); }
            g.setColour(lit ? juce::Colour(0xffff6a5e) : juce::Colour(0xff4a1814));
            g.fillEllipse(dx - dr, slot.getCentreY() - dr, dr * 2.0f, dr * 2.0f);
        }
        const float tw = (float) h * 1.12f, th2 = (float) h * 0.95f;
        metal(juce::Rectangle<float>(sliderPos - tw * 0.5f, (float) y + ((float) h - th2) * 0.5f, tw, th2), th2 * 0.22f, false);
        return;
    }
    const float tr = (float) getSliderThumbRadius(s);
    metal(juce::Rectangle<float>((float) x + (float) w * 0.02f, sliderPos - tr, (float) w * 0.96f, tr * 2.0f), tr * 0.55f, true);
}

void ArtLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box)
{
    const juce::Colour patch((juce::uint32) (int) box.getProperties()["patch"]);
    g.setColour(patch);
    g.fillRoundedRectangle(0.0f, 0.0f, (float) w, (float) h, (float) h * 0.14f);
}

void ArtLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(4, 1, box.getWidth() - 6, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
}

juce::Font ArtLookAndFeel::getComboBoxFont(juce::ComboBox& box) { return juce::Font((float) box.getHeight() * 0.52f); }

void ArtLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    const auto r = b.getLocalBounds().toFloat().reduced(1.0f);
    const bool on = b.getToggleState();
    const float cr = r.getHeight() * 0.3f;
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3c3432), 0, r.getY(), juce::Colour(0xff1c1615), 0, r.getBottom(), false));
    g.fillRoundedRectangle(r, cr);
    g.setColour(juce::Colour(on ? 0xff7a3430 : 0xff4a3c3a)); g.drawRoundedRectangle(r, cr, r.getHeight() * 0.05f);
    g.setColour(juce::Colour(on ? 0xffe8d4ce : 0xff8c7a76));
    g.setFont(juce::Font(r.getHeight() * 0.46f));
    g.drawText(b.getButtonText(), r.withTrimmedLeft(r.getHeight() * 0.3f).withTrimmedRight(r.getHeight() * 0.95f).toNearestInt(), juce::Justification::centredLeft);
    const float lr = r.getHeight() * 0.32f, lx = r.getRight() - r.getHeight() * 0.62f, ly = r.getCentreY();
    if (on) { g.setColour(ui::kGold.withAlpha(0.35f)); g.fillEllipse(lx - lr * 1.7f, ly - lr * 1.7f, lr * 3.4f, lr * 3.4f); }
    g.setGradientFill(juce::ColourGradient(juce::Colour(on ? 0xffff9c90 : 0xff5a2a26), lx - lr * 0.3f, ly - lr * 0.4f, juce::Colour(on ? 0xffe0483c : 0xff301412), lx + lr, ly + lr, true));
    g.fillEllipse(lx - lr, ly - lr, lr * 2.0f, lr * 2.0f);
}

// list geometry in "design units": the component is 85 units tall
static constexpr float kListUnits = 85.0f, kListHeader = 17.0f;

void ModeList::paint(juce::Graphics& g)
{
    const float s = (float) getHeight() / kListUnits;
    g.fillAll(juce::Colour(0xff160a0a));
    g.setColour(juce::Colour(0xffec6a5e)); g.setFont(juce::Font(8.2f * s, juce::Font::bold));
    g.drawText("DOUBLER MODE", juce::Rectangle<float>(6.0f * s, 1.0f * s, (float) getWidth(), 14.0f * s), juce::Justification::centredLeft);
    juce::Path tri; tri.addTriangle((float) getWidth() - 15.0f * s, 5.5f * s, (float) getWidth() - 6.0f * s, 5.5f * s, (float) getWidth() - 10.5f * s, 11.0f * s);
    g.setColour(juce::Colour(0xffff5a4e)); g.fillPath(tri);
    g.setColour(juce::Colour(0xff4c1c18)); g.fillRect(3.0f * s, (kListHeader - 1.0f) * s, (float) getWidth() - 6.0f * s, 1.0f);
    const auto names = dsp2::choiceNames(dsp2::P_DblMode);
    const float rowH = (kListUnits - kListHeader - 1.0f) * s / (float) names.size();
    for (size_t i = 0; i < names.size(); ++i)
    {
        const auto row = juce::Rectangle<float>(3.0f * s, kListHeader * s + (float) i * rowH, (float) getWidth() - 6.0f * s, rowH - 1.0f * s);
        const bool sel = (int) i == selected;
        if (sel) { g.setColour(juce::Colour(0xffe8383a)); g.fillRoundedRectangle(row, 2.5f * s); }
        g.setColour(sel ? juce::Colour(0xff1c0505) : juce::Colour(0xfff09284));
        g.setFont(juce::Font(9.6f * s, sel ? juce::Font::bold : juce::Font::plain));
        g.drawText(juce::String(names[i]), row.withTrimmedLeft(5.0f * s), juce::Justification::centredLeft);
    }
}

void ModeList::mouseDown(const juce::MouseEvent& e)
{
    const float s = (float) getHeight() / kListUnits;
    const int n = (int) dsp2::choiceNames(dsp2::P_DblMode).size();
    const float rowH = (kListUnits - kListHeader - 1.0f) * s / (float) n;
    const int i = (int) std::floor((e.position.y - kListHeader * s) / rowH);
    if (i >= 0 && i < n) { setSelected(i); if (onSelect) onSelect(i); }
}

// ===================================================================== small controls
static int decimalsFor(const ParamDef& d)
{
    if (d.step >= 1.0f) return 0;
    if (d.step >= 0.1f) return 1;
    return 2;
}

KnobComp::KnobComp(juce::AudioProcessorValueTreeState& s, int pi, const juce::String& caption)
{
    const ParamDef& d = paramDef(pi);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 15);
    slider.setNumDecimalPlacesToDisplay(decimalsFor(d));
    slider.setTextValueSuffix(juce::String(" ") + d.unit);
    slider.setDoubleClickReturnValue(true, d.df);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible(slider);
    label.setText(caption.isEmpty() ? juce::String(d.name) : caption, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(11.5f));
    label.setColour(juce::Label::textColourId, ui::kDim);
    addAndMakeVisible(label);
    att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(s, d.id, slider);
}
void KnobComp::resized()
{
    auto r = getLocalBounds();
    label.setBounds(r.removeFromTop(15));
    slider.setBounds(r);
}

ToggleComp::ToggleComp(juce::AudioProcessorValueTreeState& s, int pi, const juce::String& caption)
{
    const ParamDef& d = paramDef(pi);
    button.setButtonText(caption.isEmpty() ? juce::String(d.name) : caption);
    addAndMakeVisible(button);
    att = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(s, d.id, button);
}

ComboComp::ComboComp(juce::AudioProcessorValueTreeState& s, int pi, const juce::String& caption)
{
    const ParamDef& d = paramDef(pi);
    label.setText(caption.isEmpty() ? juce::String(d.name) : caption, juce::dontSendNotification);
    label.setFont(juce::Font(11.5f));
    label.setColour(juce::Label::textColourId, ui::kDim);
    addAndMakeVisible(label);
    juce::StringArray names;
    for (auto& n : choiceNames(pi)) names.add(juce::String(n));
    box.addItemList(names, 1);
    addAndMakeVisible(box);
    att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(s, d.id, box);
}
void ComboComp::resized()
{
    auto r = getLocalBounds();
    label.setBounds(r.removeFromTop(14));
    box.setBounds(r);
}

// ===================================================================== panel
void Panel::paint(juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(ui::kPanel); g.fillRoundedRectangle(r, 10.0f);
    g.setGradientFill(juce::ColourGradient(ui::kGold, r.getX(), r.getY(), ui::kMagenta.withAlpha(0.6f), r.getRight(), r.getBottom(), false));
    g.drawRoundedRectangle(r, 10.0f, 1.4f);
    g.setColour(ui::kGold);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText(titleText, 14, 6, 200, 18, juce::Justification::centredLeft);
}

void Panel::resized()
{
    auto r = getLocalBounds().reduced(8);
    auto head = r.removeFromTop(22);
    extra.setBounds(head.removeFromRight(130));
    if (!strip.empty())
    {
        auto row = r.removeFromTop(38);
        row.removeFromTop(2);
        const int n = (int) strip.size();
        int wideCount = (int) stripWide.size();
        const int smallW = 90;
        const int wideW = wideCount > 0 ? std::max(110, (row.getWidth() - (n - wideCount) * (smallW + 6)) / wideCount - 6) : 0;
        for (int i = 0; i < n; ++i)
        {
            const bool wide = std::find(stripWide.begin(), stripWide.end(), (size_t) i) != stripWide.end();
            auto cell = row.removeFromLeft(wide ? wideW : smallW);
            strip[(size_t) i]->setBounds(wide ? cell : cell.withTrimmedTop(16).withHeight(22));
            row.removeFromLeft(6);
        }
    }
    const int n = (int) knobs.size();
    if (n == 0) return;
    const int maxPer = std::max(1, r.getWidth() / 62);
    const int rowsN = (n + maxPer - 1) / maxPer;
    const int perRow = (n + rowsN - 1) / rowsN;
    const int cellH = r.getHeight() / rowsN;
    for (int i = 0; i < n; ++i)
    {
        const int row = i / perRow, col = i % perRow;
        const int inRow = std::min(perRow, n - row * perRow);
        const int cellW = r.getWidth() / inRow;
        knobs[(size_t) i]->setBounds(r.getX() + col * cellW, r.getY() + row * cellH, cellW, cellH);
    }
}

// ===================================================================== open map (every parameter)
MapContent::MapContent(juce::AudioProcessorValueTreeState& s)
{
    juce::String lastGroup;
    for (int i = 0; i < kNumParams; ++i)
    {
        const ParamDef& d = paramDef(i);
        if (lastGroup != d.group)
        {
            auto h = std::make_unique<Row>();
            h->header = 1; h->text = d.group;
            h->label = std::make_unique<juce::Label>();
            h->label->setText(d.group, juce::dontSendNotification);
            h->label->setFont(juce::Font(15.0f, juce::Font::bold));
            h->label->setColour(juce::Label::textColourId, ui::kGold);
            addAndMakeVisible(*h->label);
            rows.push_back(std::move(h));
            lastGroup = d.group;
        }
        auto row = std::make_unique<Row>();
        row->label = std::make_unique<juce::Label>();
        row->label->setText(d.name, juce::dontSendNotification);
        row->label->setColour(juce::Label::textColourId, ui::kText);
        addAndMakeVisible(*row->label);
        if (d.kind == KBool)
        {
            row->toggle = std::make_unique<juce::ToggleButton>();
            row->toggle->setButtonText("ON");
            addAndMakeVisible(*row->toggle);
            row->ba = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(s, d.id, *row->toggle);
        }
        else if (d.kind == KChoice)
        {
            row->combo = std::make_unique<juce::ComboBox>();
            juce::StringArray names; for (auto& n : choiceNames(i)) names.add(juce::String(n));
            row->combo->addItemList(names, 1);
            addAndMakeVisible(*row->combo);
            row->ca = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(s, d.id, *row->combo);
        }
        else
        {
            row->slider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
            row->slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 90, 20);
            row->slider->setNumDecimalPlacesToDisplay(decimalsFor(d));
            row->slider->setTextValueSuffix(juce::String(" ") + d.unit);
            row->slider->setDoubleClickReturnValue(true, d.df);
            addAndMakeVisible(*row->slider);
            row->sa = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(s, d.id, *row->slider);
        }
        rows.push_back(std::move(row));
    }
    totalH = (int) rows.size() * 28 + 16;
}

void MapContent::resized()
{
    int y = 8;
    const int w = getWidth();
    for (auto& r : rows)
    {
        if (r->header) { r->label->setBounds(10, y + 4, w - 20, 22); y += 28; continue; }
        r->label->setBounds(10, y, 230, 24);
        const juce::Rectangle<int> c(250, y, w - 270, 24);
        if (r->slider) r->slider->setBounds(c);
        if (r->toggle) r->toggle->setBounds(c.withWidth(70));
        if (r->combo)  r->combo->setBounds(c.withWidth(260));
        y += 28;
    }
}

// ===================================================================== editor
static juce::String noteName(float cents, float& off)
{
    static const char* n[12] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
    const int semis = (int) std::floor(cents / 100.0f + 0.5f);
    off = cents - 100.0f * (float) semis;
    const int pc = ((semis % 12) + 12) % 12;
    return juce::String(n[pc]) + juce::String(semis / 12 - 1);
}

DiscoSauceEditor::DiscoSauceEditor(DiscoSauceProcessor& p) : AudioProcessorEditor(&p), proc(p)
{
    setLookAndFeel(&laf);
    setResizable(true, true);

#if DS_HAS_ARTWORK
    {
        int artSize = 0;
        const char* artData = BinaryData::getNamedResource(BinaryData::namedResourceList[0], artSize);
        artwork = juce::ImageFileFormat::loadFrom(artData, (size_t) artSize);
    }
#endif

    auto& s = proc.apvts;

    // ---------------- artwork screen. All coordinates are pixels of the 1380 x 752 design.
    struct Spot { int param; float cx, cy, size; };
    const Spot spots[] = {
        { P_Dbl,      288, 291, 112 },   // DOUBLER        (size = 2.15 x the measured outer radius of the knob ring)
        { P_CpMix,    400, 402, 125 },   // COMPRESSOR
        { P_RvMix,    695, 401, 125 },   // REVERB
        { P_DlMix,    391, 546, 129 },   // DELAY
        { P_HsGain,   629, 548, 133 },   // EQUALIZER (air)
        { P_TuneAmt,  856, 548, 133 } }; // AUTO-TUNE
    for (auto& sp : spots)
    {
        auto k = std::make_unique<ArtKnob>();
        k->param = sp.param; k->cx = sp.cx; k->cy = sp.cy; k->size = sp.size;
        const ParamDef& d = paramDef(sp.param);
        k->s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        k->s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        k->s.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
        k->s.setLookAndFeel(&artLaf);
        k->s.setPopupDisplayEnabled(true, true, this);
        k->s.setNumDecimalPlacesToDisplay(d.step >= 1.0f ? 0 : 1);
        k->s.setTextValueSuffix(juce::String(" ") + d.unit);
        k->s.setDoubleClickReturnValue(true, d.df);
        addAndMakeVisible(k->s);
        k->a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(s, d.id, k->s);
        artKnobs.push_back(std::move(k));
    }
    dryWet.setSliderStyle(juce::Slider::LinearVertical);
    dryWet.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    dryWet.setLookAndFeel(&artLaf);
    dryWet.setPopupDisplayEnabled(true, true, this);
    dryWet.setTextValueSuffix(" %");
    dryWet.setNumDecimalPlacesToDisplay(0);
    dryWet.setDoubleClickReturnValue(true, 100.0);
    addAndMakeVisible(dryWet);
    dryWetAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(s, paramDef(P_Mix).id, dryWet);

    {
        juce::StringArray kn, sn;
        for (auto& n : choiceNames(P_Key)) kn.add(juce::String(n));
        for (auto& n : choiceNames(P_Scale)) sn.add(juce::String(n));
        keyBox.addItemList(kn, 1); scaleBox.addItemList(sn, 1);
        keyBox.setLookAndFeel(&artLaf); scaleBox.setLookAndFeel(&artLaf);
        keyBox.getProperties().set("patch", (int) 0xff220c0b);
        scaleBox.getProperties().set("patch", (int) 0xff220c0b);
        addAndMakeVisible(keyBox); addAndMakeVisible(scaleBox);
        keyAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(s, paramDef(P_Key).id, keyBox);
        scaleAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(s, paramDef(P_Scale).id, scaleBox);
        tuneToggle.setLookAndFeel(&artLaf);
        addAndMakeVisible(tuneToggle);
        tuneAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(s, paramDef(P_TuneOn).id, tuneToggle);
    }
    modeList.onSelect = [this](int m) { proc.applyDoublerMode(m); };
    addAndMakeVisible(modeList);

    // ---------------- detailed panels screen
    auto add = [&](const juce::String& title) -> Panel& { panels.push_back(std::make_unique<Panel>(s, title)); mainPage.addAndMakeVisible(*panels.back()); return *panels.back(); };
    { auto& a = add("AUTO-TUNE");
      a.addToggle(P_TuneOn, "TUNE ON"); a.addCombo(P_Key, "Key"); a.addCombo(P_Scale, "Scale");
      a.addKnob(P_TuneAmt, "Correction"); a.addKnob(P_Retune, "Retune"); a.addKnob(P_Humanize); a.addKnob(P_Hyst, "Stickiness"); a.addKnob(P_Transpose); a.addKnob(P_Fine); }
    { auto& d = add("DOUBLER");
      d.addCombo(P_DblMode, "Mode");
      d.addKnob(P_Dbl, "DOUBLER"); d.addKnob(P_DblVoices, "Voices"); d.addKnob(P_DblDelay, "Delay"); d.addKnob(P_DblDetune, "Detune"); d.addKnob(P_DblWidth, "Width"); d.addKnob(P_DblTone, "Tone"); d.addKnob(P_DblOct, "Octave Up"); }
    { auto& c = add("COMPRESSOR"); compPanel = &c; c.enableExtra();
      c.addToggle(P_CpOn, "ON"); c.addKnob(P_CpThr, "Threshold"); c.addKnob(P_CpRatio, "Ratio"); c.addKnob(P_CpAtk, "Attack"); c.addKnob(P_CpRel, "Release"); c.addKnob(P_CpMake, "Makeup"); c.addKnob(P_CpMix, "Mix"); }
    { auto& e = add("EQUALIZER");
      e.addToggle(P_EqOn, "ON"); e.addKnob(P_HpFreq, "Low Cut"); e.addKnob(P_LsGain, "Low Shelf"); e.addKnob(P_B1G, "Bell 1"); e.addKnob(P_B2G, "Bell 2"); e.addKnob(P_B3G, "Bell 3"); e.addKnob(P_HsGain, "High Shelf"); }
    { auto& d = add("DE-ESSER");
      d.addToggle(P_DsOn, "ON"); d.addKnob(P_DsFreq, "Freq"); d.addKnob(P_DsThr, "Threshold"); d.addKnob(P_DsAmt, "Amount"); }
    { auto& d = add("SATURATION");
      d.addToggle(P_SatOn, "ON"); d.addKnob(P_SatDrive, "Drive"); d.addKnob(P_SatMix, "Mix"); }
    { auto& d = add("DELAY");
      d.addToggle(P_DlOn, "ON"); d.addToggle(P_DlSync, "SYNC"); d.addToggle(P_DlPing, "PING-PONG");
      d.addKnob(P_DlMix, "Mix"); d.addKnob(P_DlTime, "Time"); d.addKnob(P_DlFb, "Feedback"); d.addKnob(P_DlTone, "Tone"); }
    { auto& d = add("REVERB");
      d.addToggle(P_RvOn, "ON"); d.addKnob(P_RvMix, "Mix"); d.addKnob(P_RvSize, "Size"); d.addKnob(P_RvDamp, "Damping"); d.addKnob(P_RvPre, "Pre-Delay"); d.addKnob(P_RvWidth, "Width"); }
    { auto& d = add("OUTPUT");
      d.addToggle(P_LimOn, "LIMITER"); d.addKnob(P_InGain, "Input"); d.addKnob(P_OutGain, "Output"); d.addKnob(P_Mix, "Dry/Wet"); }
    addChildComponent(mainPage);

    mapContent = std::make_unique<MapContent>(s);
    mapViewport.setViewedComponent(mapContent.get(), false);
    mapViewport.setScrollBarsShown(true, false);
    addChildComponent(mapViewport);

    // ---------------- shared controls
    for (size_t i = 0; i < presets().size(); ++i) presetBox.addItem(presets()[i].name, (int) i + 1);
    presetBox.setSelectedId(proc.presetIndex.load() + 1, juce::dontSendNotification);
    presetBox.getProperties().set("patch", (int) 0xff470b0b);
    presetBox.onChange = [this] { presetChanged(); };
    addAndMakeVisible(presetBox);

    presetAmount.setSliderStyle(juce::Slider::LinearHorizontal);
    presetAmount.setRange(0.0, 100.0, 1.0);
    presetAmount.setTextValueSuffix(" %");
    presetAmount.setValue(proc.presetAmount.load() * 100.0f, juce::dontSendNotification);
    presetAmount.onValueChange = [this]
    {
        proc.applyPreset(presetBox.getSelectedId() - 1, (float) presetAmount.getValue() * 0.01f);
        repaint(juce::Rectangle<int>((int) (354 * kScale), (int) (114 * kScale), (int) (102 * kScale), (int) (38 * kScale)));
    };
    addAndMakeVisible(presetAmount);

    panelsButton.onClick = [this] { setView(view == ViewPanels ? ViewArt : ViewPanels); };
    mapButton.onClick    = [this] { setView(view == ViewMap ? ViewArt : ViewMap); };
    addAndMakeVisible(panelsButton);
    addAndMakeVisible(mapButton);

    saveButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser>("Save preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.dspreset");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
            [this](const juce::FileChooser& fc) { auto f = fc.getResult(); if (f != juce::File()) proc.savePresetFile(f.withFileExtension("dspreset")); });
    };
    loadButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser>("Load preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.dspreset");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) { auto f = fc.getResult(); if (f.existsAsFile()) proc.loadPresetFile(f); });
    };
    addChildComponent(saveButton);
    addChildComponent(loadButton);

    view = -1;
    setView(ViewArt);
    startTimerHz(20);
}

DiscoSauceEditor::~DiscoSauceEditor()
{
    stopTimer();
    for (auto& k : artKnobs) k->s.setLookAndFeel(nullptr);
    dryWet.setLookAndFeel(nullptr);
    keyBox.setLookAndFeel(nullptr); scaleBox.setLookAndFeel(nullptr); tuneToggle.setLookAndFeel(nullptr);
    presetBox.setLookAndFeel(nullptr); presetAmount.setLookAndFeel(nullptr);
    panelsButton.setLookAndFeel(nullptr); mapButton.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void DiscoSauceEditor::presetChanged()
{
    proc.applyPreset(presetBox.getSelectedId() - 1, (float) presetAmount.getValue() * 0.01f);
}

void DiscoSauceEditor::setView(int v)
{
    if (v == view) return;
    view = v;
    const bool art = (v == ViewArt);
    for (auto& k : artKnobs) k->s.setVisible(art);
    dryWet.setVisible(art); keyBox.setVisible(art); scaleBox.setVisible(art); tuneToggle.setVisible(art); modeList.setVisible(art);
    mainPage.setVisible(v == ViewPanels);
    mapViewport.setVisible(v == ViewMap);
    saveButton.setVisible(!art); loadButton.setVisible(!art);

    // shared controls: transparent "ghost" look on the artwork, normal look on the detail screens
    juce::LookAndFeel* lf = art ? static_cast<juce::LookAndFeel*>(&artLaf) : static_cast<juce::LookAndFeel*>(&laf);
    presetBox.setLookAndFeel(lf); presetAmount.setLookAndFeel(lf);
    panelsButton.setLookAndFeel(lf); mapButton.setLookAndFeel(lf);
    presetAmount.setTextBoxStyle(art ? juce::Slider::NoTextBox : juce::Slider::TextBoxRight, false, 52, 18);
    panelsButton.setButtonText(v == ViewPanels ? "BACK" : "PRO PANELS");
    mapButton.setButtonText(v == ViewMap ? "BACK" : "OPEN MIXER");

    if (art)
    {
        getConstrainer()->setFixedAspectRatio(1380.0 / 752.0);
        setResizeLimits(690, 376, 3840, 2093);
        setSize(1300, 708);
    }
    else
    {
        getConstrainer()->setFixedAspectRatio(0.0);
        setResizeLimits(900, 640, 3000, 2200);
        setSize(1100, 780);
    }
    resized();
    repaint();
}

void DiscoSauceEditor::timerCallback()
{
    if (compPanel != nullptr)
        compPanel->extra.setText("GR " + juce::String(proc.meterGr.load(), 1) + " dB", juce::dontSendNotification);
    modeList.setSelected((int) proc.apvts.getRawParameterValue(paramDef(P_DblMode).id)->load());
    tunerVoiced = proc.meterVoiced.load();
    float off = 0.0f, off2 = 0.0f;
    tunerIn = noteName(proc.meterInCents.load(), off);
    tunerOut = noteName(proc.meterTgtCents.load(), off2);
    tunerCents = off;
    if (view == ViewArt)
        repaint(juce::Rectangle<int>((int) (934 * kScale), (int) (58 * kScale), (int) (336 * kScale), (int) (86 * kScale)));
    else
        repaint(0, 0, getWidth(), 116);
}

void DiscoSauceEditor::paintTuner(juce::Graphics& g)      // drawn in design pixels (1380 x 752)
{
    g.setColour(juce::Colour(0xff1a0b0a)); g.fillRoundedRectangle(940.0f, 65.0f, 321.0f, 73.0f, 6.0f);
    g.setColour(juce::Colour(0xffe8d6d0)); g.setFont(juce::Font(11.5f));
    g.drawText("TUNER", 954, 70, 60, 14, juce::Justification::centredLeft);

    const int bars = 25; const float bx0 = 956.0f, bx1 = 1214.0f, base = 118.0f;
    const float cursor = (juce::jlimit(-50.0f, 50.0f, tunerCents) + 50.0f) / 100.0f * (float) (bars - 1);
    for (int i = 0; i < bars; ++i)
    {
        const float x = bx0 + (bx1 - bx0) * (float) i / (float) (bars - 1);
        const float hgt = 13.0f + 37.0f * (float) i / (float) (bars - 1);
        const bool lit = tunerVoiced && std::abs((float) i - cursor) <= 1.0f;
        const bool nearLit = tunerVoiced && std::abs((float) i - cursor) <= 2.0f;
        if (lit) { g.setColour(ui::kGold.withAlpha(0.3f)); g.fillRoundedRectangle(x - 5.0f, base - hgt - 3.0f, 10.0f, hgt + 6.0f, 3.0f); }
        g.setColour(lit ? juce::Colour(0xffff6c60) : (nearLit ? juce::Colour(0xffa83028) : juce::Colour(0xff4e1512)));
        g.fillRoundedRectangle(x - 2.6f, base - hgt, 5.2f, hgt, 1.6f);
    }
    g.setColour(juce::Colour(0xffe8d6d0)); g.fillRect(1087.0f, 68.0f, 1.2f, 52.0f);          // centre line = in tune
    g.setFont(juce::Font(11.0f));
    g.drawText("-50", 954, 122, 40, 14, juce::Justification::centredLeft);
    g.drawText("0", 1068, 122, 40, 14, juce::Justification::centred);
    g.drawText("+50", 1180, 122, 40, 14, juce::Justification::centredRight);

    g.setColour(juce::Colour(tunerVoiced ? 0xffff6c60 : 0xff6a2420)); g.setFont(juce::Font(22.0f, juce::Font::bold));
    g.drawText(tunerVoiced ? juce::String(juce::roundToInt(tunerCents)) : juce::String("--"), 1219, 86, 40, 26, juce::Justification::centred);
    g.setColour(juce::Colour(0xffd8c2bc)); g.setFont(juce::Font(10.0f));
    g.drawText("cents", 1219, 114, 40, 12, juce::Justification::centred);
    if (tunerVoiced)
    {
        g.setColour(juce::Colour(0xfff6e4de)); g.setFont(juce::Font(14.5f, juce::Font::bold));
        g.drawText(tunerIn + "  ->  " + tunerOut, 1000, 69, 110, 17, juce::Justification::centredLeft);
    }
}

void DiscoSauceEditor::paint(juce::Graphics& g)
{
    g.fillAll(ui::kBg);
    if (view == ViewArt)
    {
        if (artwork.isValid())
        {
            g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
            g.drawImage(artwork, getLocalBounds().toFloat());
        }
        g.saveState();
        g.addTransform(juce::AffineTransform::scale(kScale));
        g.setColour(juce::Colour(0xff360b0a)); g.fillRoundedRectangle(362.0f, 121.0f, 86.0f, 24.0f, 3.0f);
        g.setColour(juce::Colour(0xffffa093)); g.setFont(juce::Font(19.0f));
        g.drawText(juce::String((int) presetAmount.getValue()) + " %", juce::Rectangle<int>(360, 119, 90, 28), juce::Justification::centred);
        paintTuner(g);
        g.restoreState();
    }
    else
    {
        g.setColour(juce::Colour(0xff120505)); g.fillRect(0, 0, getWidth(), 116);
        g.setColour(ui::kGold); g.setFont(juce::Font(48.0f, juce::Font::bold));
        g.drawText("DISCO SAUCE", 20, 6, 420, 60, juce::Justification::centredLeft);
        g.setColour(ui::kCyan); g.setFont(juce::Font(15.0f, juce::Font::bold));
        g.drawText("ALL-IN-ONE  2 PRO  -  flowmotion", 24, 70, 360, 20, juce::Justification::centredLeft);
        g.setColour(ui::kDim); g.setFont(juce::Font(12.0f));
        g.drawText("Latency " + juce::String(proc.getLatencySamples()) + " samples (reported to host)", 24, 94, 360, 16, juce::Justification::centredLeft);
        g.setColour(ui::kText); g.setFont(juce::Font(22.0f, juce::Font::bold));
        g.drawText(tunerVoiced ? (tunerIn + "   " + juce::String(tunerCents, 0) + " c   ->  " + tunerOut) : juce::String("--"), 470, 30, 330, 40, juce::Justification::centredLeft);
        g.setColour(ui::kDim); g.setFont(juce::Font(11.0f)); g.drawText("TUNER", 470, 12, 100, 16, juce::Justification::centredLeft);
    }
}

void DiscoSauceEditor::layoutArt()
{
    const float k = (float) getWidth() / 1380.0f;
    kScale = k;
    auto R = [k](float x, float y, float w, float h) { return juce::Rectangle<int>((int) (x * k), (int) (y * k), (int) (w * k), (int) (h * k)); };
    for (auto& a : artKnobs)
    {
        const float sz = a->size;
        a->s.setBounds(R(a->cx - sz * 0.5f, a->cy - sz * 0.5f, sz, sz));
    }
    dryWet.setBounds(R(1090, 361, 44, 224));
    presetBox.setBounds(R(168, 74, 252, 31));
    presetAmount.setBounds(R(160, 120, 194, 32));
    tuneToggle.setBounds(R(157, 157, 101, 28));
    keyBox.setBounds(R(272, 157, 40, 26));
    scaleBox.setBounds(R(314, 157, 108, 26));
    panelsButton.setBounds(R(935, 148, 148, 31));
    mapButton.setBounds(R(1093, 148, 140, 30));
    modeList.setBounds(R(1068, 245, 124, 85));
}

void DiscoSauceEditor::layoutPanels()
{
    kScale = 1.0f;
    auto r = getLocalBounds();
    auto head = r.removeFromTop(116);
    auto right = head.removeFromRight(300).reduced(6, 8);
    presetBox.setBounds(right.removeFromTop(26));
    right.removeFromTop(6);
    presetAmount.setBounds(right.removeFromTop(24));
    right.removeFromTop(6);
    auto btns = right.removeFromTop(24);
    panelsButton.setBounds(btns.removeFromLeft(96)); btns.removeFromLeft(4);
    mapButton.setBounds(btns.removeFromLeft(96)); btns.removeFromLeft(4);
    saveButton.setBounds(btns.removeFromLeft(40)); btns.removeFromLeft(4);
    loadButton.setBounds(btns.removeFromLeft(40));

    auto body = r.reduced(6, 4);
    mapViewport.setBounds(body);
    mapContent->setSize(std::max(200, body.getWidth() - 18), mapContent->requiredHeight());
    mainPage.setBounds(body);

    const int rowH = body.getHeight() / 3;
    struct Cell { int idx; int w; };
    const std::vector<std::vector<Cell>> layout = {
        { {0, 560}, {1, 440} },
        { {2, 360}, {3, 400}, {4, 240} },
        { {5, 170}, {6, 300}, {7, 300}, {8, 230} } };
    for (int row = 0; row < 3; ++row)
    {
        int x = 0;
        for (auto& c : layout[(size_t) row])
        {
            const int w = body.getWidth() * c.w / 1000;
            panels[(size_t) c.idx]->setBounds(x, row * rowH, w, rowH);
            x += w;
        }
    }
}

void DiscoSauceEditor::resized()
{
    if (view == ViewArt) layoutArt(); else layoutPanels();
}
