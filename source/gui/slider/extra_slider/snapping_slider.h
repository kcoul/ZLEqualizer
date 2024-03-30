// ==============================================================================
// Copyright (C) 2024 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#ifndef SNAPPING_SLIDER_H
#define SNAPPING_SLIDER_H

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlInterface {
    class SnappingSlider final : public juce::Slider {
    public:
        SnappingSlider() : juce::Slider() {
        }

        void setSnappingVal(const float val) noexcept { m_snapVal = val; }

        void mouseWheelMove(const juce::MouseEvent &e, const juce::MouseWheelDetails &wheel) override {
            w = wheel;
            if (e.mods.isCommandDown()) {
                w.deltaX *= m_snapVal;
                w.deltaY *= m_snapVal;
            }
            Slider::mouseWheelMove(e, w);
        }

    private:
        float m_snapVal{0.125f};
        juce::MouseWheelDetails w{0.f, 0.f, true, true, true};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SnappingSlider)
    };
}
#endif //SNAPPING_SLIDER_H
