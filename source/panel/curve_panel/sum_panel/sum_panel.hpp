// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_SUM_PANEL_HPP
#define ZLEqualizer_SUM_PANEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"

namespace zlPanel {
    class SumPanel final : public juce::Component,
                           private juce::Thread,
                           private juce::AsyncUpdater,
                           private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit SumPanel(juce::AudioProcessorValueTreeState &parameters,
                          zlInterface::UIBase &base,
                          zlDSP::Controller<double> &controller);

        ~SumPanel() override;

        void paint(juce::Graphics &g) override;

        void setMaximumDB(const float x) {
            maximumDB.store(x);
            toRepaint.store(true);
        }

        void checkRepaint();

        void resized() override;

    private:
        std::array<juce::Path, 5> paths;
        juce::AudioProcessorValueTreeState &parametersRef;
        zlInterface::UIBase &uiBase;
        zlDSP::Controller<double> &c;
        std::atomic<float> maximumDB;
        std::array<juce::CriticalSection, 5> pathUpdateLocks;

        static constexpr std::array changeIDs{
            // zlDSP::fType::ID, zlDSP::slope::ID,
            zlDSP::bypass::ID, zlDSP::lrType::ID,
            // zlDSP::targetGain::ID, zlDSP::targetQ::ID
        };
        std::atomic<bool> toRepaint{false};

        void run() override;

        void handleAsyncUpdate() override;

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
} // zlPanel

#endif //ZLEqualizer_SUM_PANEL_HPP
