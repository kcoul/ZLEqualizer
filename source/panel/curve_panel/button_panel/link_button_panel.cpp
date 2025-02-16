// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "link_button_panel.hpp"

namespace zlPanel {
    LinkButtonPanel::LinkButtonPanel(const size_t idx,
                                     juce::AudioProcessorValueTreeState &parameters,
                                     juce::AudioProcessorValueTreeState &parametersNA,
                                     zlInterface::UIBase &base,
                                     zlInterface::Dragger &sideDragger)
        : parametersRef(parameters), parametersNARef(parametersNA),
          uiBase(base),
          sideDraggerRef(sideDragger),
          dynLinkC("L", base),
          linkDrawable(juce::Drawable::createFromImageData(BinaryData::linksfill_svg, BinaryData::linksfill_svgSize)),
          bandIdx(idx) {
        dynLinkC.getLAF().enableShadow(false);
        dynLinkC.setDrawable(linkDrawable.get());
        attach({&dynLinkC.getButton()}, {zlDSP::appendSuffix(zlDSP::singleDynLink::ID, bandIdx)},
               parameters, buttonAttachments);
        addChildComponent(dynLinkC);
        setInterceptsMouseClicks(false, true);

        for (auto &ID: IDs) {
            const auto suffixID = zlDSP::appendSuffix(ID, idx);
            parametersRef.addParameterListener(suffixID, this);
            parameterChanged(suffixID, parametersRef.getRawParameterValue(suffixID)->load());
        }
        for (auto &ID: NAIDs) {
            parametersNARef.addParameterListener(ID, this);
            parameterChanged(ID, parametersNARef.getRawParameterValue(ID)->load());
        }
    }

    LinkButtonPanel::~LinkButtonPanel() {
        const auto idx = bandIdx.load();
        for (auto &ID: IDs) {
            parametersRef.removeParameterListener(zlDSP::appendSuffix(ID, idx), this);
        }
        for (auto &ID: NAIDs) {
            parametersNARef.removeParameterListener(ID, this);
        }
    }

    void LinkButtonPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID.startsWith(zlDSP::sideFreq::ID)) {
            sideFreq.store(newValue);
        } else if (parameterID.startsWith(zlDSP::dynamicON::ID)) {
            isDynamicON.store(newValue > .5f);
        } else if (parameterID.startsWith(zlState::selectedBandIdx::ID)) {
            isSelected.store(static_cast<size_t>(newValue) == bandIdx.load());
        }
    }

    void LinkButtonPanel::updateBound() {
        if (isSelected.load() && isDynamicON.load()) {
            const auto dynPos = static_cast<float>(sideDraggerRef.getButton().getBounds().getCentreX());
            buttonBound = juce::Rectangle<float>{2.5f * uiBase.getFontSize(), 2.5f * uiBase.getFontSize()};
            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 8 * uiBase.getFontSize());
            buttonBound = buttonBound.withCentre(
                {dynPos, bound.getBottom()}
            );
            dynLinkC.setBounds(buttonBound.toNearestInt());
            dynLinkC.setVisible(true);
        } else {
            dynLinkC.setVisible(false);
        }
    }
} // zlPanel
