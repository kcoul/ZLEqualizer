#ifndef COMPACT_COMBOBOX_LOOK_AND_FEEL_H
#define COMPACT_COMBOBOX_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class CompactComboboxLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        // rounded menu box
        explicit CompactComboboxLookAndFeel(UIBase &base) {
            uiBase = &base;
            setColour(juce::PopupMenu::backgroundColourId, uiBase->getBackgroundInactiveColor());
        }

        void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown, int, int, int, int,
                          juce::ComboBox &box) override {
            juce::ignoreUnused(width, height);
            const auto boxBounds = juce::Rectangle<float>(0, 0,
                                                          static_cast<float>(width),
                                                          static_cast<float>(height));
            const auto cornerSize = uiBase->getFontSize() * 0.375f;
            if (isButtonDown || box.isPopupActive()) {
                g.setColour(uiBase->getTextInactiveColor());
                g.fillRoundedRectangle(boxBounds, cornerSize);
            } else {
                uiBase->fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize,
                                                        {
                                                            .blurRadius = 0.45f, .flip = true,
                                                            .darkShadowColor = uiBase->getDarkShadowColor().
                                                            withMultipliedAlpha(boxAlpha.load()),
                                                            .brightShadowColor = uiBase->getBrightShadowColor().
                                                            withMultipliedAlpha(boxAlpha.load()),
                                                            .changeDark = true, .changeBright = true
                                                        });
            }
        }

        void positionComboBoxText(juce::ComboBox &box, juce::Label &label) override {
            label.setBounds(0, 0, box.getWidth(), box.getHeight());
        }

        void drawLabel(juce::Graphics &g, juce::Label &label) override {
            if (editable.load()) {
                g.setColour(uiBase->getTextColor());
            } else {
                g.setColour(uiBase->getTextInactiveColor());
            }
            g.setFont(uiBase->getFontSize() * FontLarge);
            g.drawText(label.getText(), label.getLocalBounds(), juce::Justification::centred);
        }

        void drawPopupMenuBackground(juce::Graphics &g, int width, int height) override {
            const auto cornerSize = uiBase->getFontSize() * 0.375f;
            const auto boxBounds = juce::Rectangle<float>(0, 0, static_cast<float>(width),
                                                          static_cast<float>(height));
            // boxBounds = uiBase->fillRoundedShadowRectangle(g, boxBounds, cornerSize,
            //                                                {.curveTopLeft = true, .curveTopRight = true});
            uiBase->fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize, {.blurRadius = 0.45f, .flip = true});
            // g.setColour(uiBase->getTextInactiveColor());
            // g.fillRect(boxBounds.getX(), 0.0f, boxBounds.getWidth(), cornerSize * 0.15f);
        }

        void getIdealPopupMenuItemSize(const juce::String &text, const bool isSeparator, int standardMenuItemHeight,
                                       int &idealWidth, int &idealHeight) override {
            juce::ignoreUnused(text, isSeparator, standardMenuItemHeight);
            idealWidth = static_cast<int>(0);
            idealHeight = static_cast<int>(uiBase->getFontSize() * FontLarge * 1.2f);
        }

        void drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area,
                               const bool isSeparator, const bool isActive,
                               const bool isHighlighted, const bool isTicked, const bool hasSubMenu,
                               const juce::String &text,
                               const juce::String &shortcutKeyText, const juce::Drawable *icon,
                               const juce::Colour *const textColourToUse) override {
            juce::ignoreUnused(isSeparator, hasSubMenu, shortcutKeyText, icon, textColourToUse);
            if ((isHighlighted || isTicked) && isActive && editable) {
                g.setColour(uiBase->getTextColor());
            } else {
                g.setColour(uiBase->getTextInactiveColor());
            }
            if (uiBase->getFontSize() > 0) {
                g.setFont(uiBase->getFontSize() * FontLarge);
            } else {
                g.setFont(static_cast<float>(area.getHeight()) * 0.35f);
            }
            g.drawText(text, area, juce::Justification::centred);
        }

        int getMenuWindowFlags() override {
            return 1;
        }

        int getPopupMenuBorderSize() override {
            return juce::roundToInt(uiBase->getFontSize() * 0.125f);
        }

        inline void setEditable(bool f) { editable.store(f); }

        inline void setBoxAlpha(const float x) { boxAlpha.store(x); }

        inline float getBoxAlpha() const { return boxAlpha.load(); }

    private:
        std::atomic<bool> editable = true;
        std::atomic<float> boxAlpha;

        UIBase *uiBase;
    };
}

#endif //COMPACT_COMBOBOX_LOOK_AND_FEEL_H
