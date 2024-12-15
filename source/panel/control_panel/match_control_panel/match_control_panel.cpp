// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_control_panel.hpp"

namespace zlPanel {
    MatchControlPanel::MatchControlPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : uiBase(base), analyzer(p.getController().getMatchAnalyzer()),
          startDrawable(juce::Drawable::createFromImageData(BinaryData::playfill_svg, BinaryData::playfill_svgSize)),
          pauseDrawable(juce::Drawable::createFromImageData(BinaryData::pauseline_svg, BinaryData::pauseline_svgSize)),
          saveDrawable(juce::Drawable::createFromImageData(BinaryData::saveline_svg, BinaryData::saveline_svgSize)),
          sideChooseBox("", {"Side", "Preset", "Flat"}, base),
          fitAlgoBox("", {"LD", "GN"}, base),
          weightSlider("Weight", base),
          smoothSlider("Smooth", base),
          slopeSlider("Slope", base),
          numBandSlider("Num Band", base),
          learnButton(base, startDrawable.get(), pauseDrawable.get()),
          saveButton(base, saveDrawable.get()),
          fitButton(base, startDrawable.get()) {
        juce::ignoreUnused(p);
        uiBase.getValueTree().addListener(this);
        // create preset directory if not exists
        if (!presetDirectory.isDirectory()) {
            const auto f = presetDirectory.createDirectory();
            juce::ignoreUnused(f);
        }
        // init combobox
        sideChooseBox.getBox().setSelectedId(1);
        sideChooseBox.getBox().onChange = [this]() {
            const auto matchMode = static_cast<zlEqMatch::EqMatchAnalyzer<double>::MatchMode>(
                sideChooseBox.getBox().getSelectedId() - 1);
            switch (matchMode) {
                case zlEqMatch::EqMatchAnalyzer<double>::matchSide: {
                    break;
                }
                case zlEqMatch::EqMatchAnalyzer<double>::matchSlope: {
                    analyzer.setTargetSlope(0.f);
                    break;
                }
                case zlEqMatch::EqMatchAnalyzer<double>::matchPreset: {
                    loadFromPreset();
                    break;
                }
            }
            analyzer.setMatchMode(matchMode);
        };
        fitAlgoBox.getBox().setSelectedId(1);
        for (const auto &c: {&sideChooseBox, &fitAlgoBox}) {
            addAndMakeVisible(c);
        }
        weightSlider.getSlider().setRange(0.0, 1.0, 0.01);
        weightSlider.getSlider().setDoubleClickReturnValue(true, .5);
        weightSlider.getSlider().onValueChange = [this]() {
            analyzer.getAverageFFT().setWeight(static_cast<float>(weightSlider.getSlider().getValue()));
        };
        smoothSlider.getSlider().setRange(0.0, 1.0, 0.01);
        smoothSlider.getSlider().setDoubleClickReturnValue(true, .5);
        smoothSlider.getSlider().onValueChange = [this]() {
            analyzer.setSmooth(static_cast<float>(smoothSlider.getSlider().getValue()));
        };
        slopeSlider.getSlider().setRange(-4.5, 4.5, 0.01);
        slopeSlider.getSlider().setDoubleClickReturnValue(true, 0.);
        slopeSlider.getSlider().onValueChange = [this]() {
            analyzer.setSlope(static_cast<float>(slopeSlider.getSlider().getValue()));
        };
        numBandSlider.getSlider().setRange(1.0, 16.0, 1.0);
        numBandSlider.getSlider().setValue(8.0);
        for (const auto &c: {&weightSlider, &smoothSlider, &slopeSlider, &numBandSlider}) {
            addAndMakeVisible(c);
        }
        for (const auto &c: {&learnButton, &saveButton, &fitButton}) {
            addAndMakeVisible(c);
            c->setPadding(.2f, .2f, .2f, .2f);
        }

        learnButton.getButton().onStateChange = [this]() {
            analyzer.setON(learnButton.getButton().getToggleState());
        };
        saveButton.getButton().onClick = [this]() {
            learnButton.getButton().setToggleState(false, juce::dontSendNotification);
            analyzer.setON(false);
            saveToPreset();
        };
        resetDefault();
    }

    MatchControlPanel::~MatchControlPanel() {
        uiBase.getValueTree().removeListener(this);
        analyzer.setON(false);
    }

    void MatchControlPanel::paint(juce::Graphics &g) {
        auto bound = getLocalBounds().toFloat();
        g.fillAll(uiBase.getColourByIdx(zlInterface::colourIdx::backgroundColour));
        bound = bound.removeFromLeft(bound.getWidth() * weightP);
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }

    void MatchControlPanel::resized() {
        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        grid.templateRows = {Track(Fr(1)), Track(Fr(1))};
        grid.templateColumns = {
            Track(Fr(60)), Track(Fr(30)), Track(Fr(60)),
            Track(Fr(30)), Track(Fr(30))
        };

        grid.items = {
            juce::GridItem(sideChooseBox).withArea(1, 1),
            juce::GridItem(weightSlider).withArea(2, 1),
            juce::GridItem(learnButton).withArea(1, 2),
            juce::GridItem(saveButton).withArea(2, 2),
            juce::GridItem(smoothSlider).withArea(1, 3),
            juce::GridItem(slopeSlider).withArea(2, 3),
            juce::GridItem(fitAlgoBox).withArea(1, 4),
            juce::GridItem(fitButton).withArea(1, 5),
            juce::GridItem(numBandSlider).withArea(2, 4, 3, 6)
        };

        for (const auto &c: {&weightSlider, &smoothSlider, &slopeSlider, &numBandSlider}) {
            c->setPadding(uiBase.getFontSize() * 0.5f, 0.f);
        }

        auto bound = getLocalBounds().toFloat();
        bound = bound.removeFromLeft(bound.getWidth() * weightP);
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});
        grid.performLayout(bound.toNearestInt());
    }

    void MatchControlPanel::resetDefault() {
        weightSlider.getSlider().setValue(0.5, juce::dontSendNotification);
        analyzer.getAverageFFT().setWeight(.5f);

        smoothSlider.getSlider().setValue(0.5, juce::dontSendNotification);
        analyzer.setSmooth(.5f);

        slopeSlider.getSlider().setValue(0., juce::dontSendNotification);
        analyzer.setSlope(0.f);

        learnButton.getButton().setToggleState(false, juce::dontSendNotification);
        analyzer.setON(false);
        analyzer.reset();
    }

    void MatchControlPanel::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &) {
        const auto f = static_cast<bool>(uiBase.getProperty(zlInterface::settingIdx::matchPanelShow));
        setVisible(f);
        if (!f) {
            resetDefault();
        }
    }

    void MatchControlPanel::loadFromPreset() {
        myChooser = std::make_unique<juce::FileChooser>(
            "Load the match preset...", presetDirectory, "*.csv",
            true, false, nullptr);
        constexpr auto settingOpenFlags = juce::FileBrowserComponent::openMode |
                                          juce::FileBrowserComponent::canSelectFiles;
        myChooser->launchAsync(settingOpenFlags, [this](const juce::FileChooser &chooser) {
            const juce::File settingFile(chooser.getResult());
            if (!settingFile.existsAsFile()) { return; }
            const auto stream(settingFile.createInputStream());
            if (stream->isExhausted()) { return; }
            const auto start = stream->readNextLine();
            if (start.startsWith("#native")) {
                std::array<float, 251> points;
                size_t idx = 0;
                while (!stream->isExhausted() && idx < points.size()) {
                    points[idx] = stream->readNextLine().getFloatValue();
                    idx += 1;
                }
                if (idx == points.size()) {
                    analyzer.setTargetPreset(points);
                }
            }
        });
    }

    void MatchControlPanel::saveToPreset() {
        myChooser = std::make_unique<juce::FileChooser>(
            "Save the match preset...", presetDirectory.getChildFile("match.csv"), "*.csv",
            true, false, nullptr);
        constexpr auto settingSaveFlags = juce::FileBrowserComponent::saveMode |
                                          juce::FileBrowserComponent::warnAboutOverwriting;
        myChooser->launchAsync(settingSaveFlags, [this](const juce::FileChooser &chooser) {
            juce::File settingFile(chooser.getResult().withFileExtension("csv"));
            if (settingFile.existsAsFile()) {
                const auto f = settingFile.deleteFile();
                juce::ignoreUnused(f);
            }
            const auto stream = settingFile.createOutputStream();
            stream->writeText("#native", false, false, nullptr);
            stream->writeText(",\n", false, false, nullptr);
            for (auto &p: analyzer.getTarget()) {
                stream->writeText(juce::String(p), false, false, nullptr);
                stream->writeText(",\n", false, false, nullptr);
            }
        });
    }
} // zlPanel
