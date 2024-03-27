// ==============================================================================
// Copyright (C) 2023 - zsliu98
// This file is part of ZLEComp
//
// ZLEComp is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// ZLEComp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEComp. If not, see <https://www.gnu.org/licenses/>.
// ==============================================================================

#include "knee_computer.hpp"

namespace zlCompressor {
    template<typename FloatType>
    KneeComputer<FloatType>::KneeComputer(const KneeComputer<FloatType> &c) {
        setThreshold(c.getThreshold());
        setRatio(c.getRatio());
        setKneeW(c.getKneeW());
        setKneeD(c.getKneeD());
        setKneeS(c.getKneeS());
        setBound(c.getBound());
    }

    template<typename FloatType>
    KneeComputer<FloatType>::~KneeComputer() = default;

    template<typename FloatType>
    FloatType KneeComputer<FloatType>::eval(FloatType x) {
        const juce::ScopedLock scopedLock(paraUpdateLock);
        const auto threshold_ = threshold.load();
        const auto kneeW_ = kneeW.load();
        const auto bound_ = bound.load();
        const auto ratio_ = ratio.load();
        if (x <= threshold_ - kneeW_) {
            return x;
        } else if (x >= threshold_ + kneeW_) {
            return juce::jlimit(x - bound_, x + bound_, threshold_ + (x - threshold_) / ratio_);
        } else {
            const auto xx = x + tempB.load();
            return juce::jlimit(x - bound_, x + bound_, x + tempA.load() * xx * xx / tempC.load());
        }
    }

    template<typename FloatType>
    FloatType KneeComputer<FloatType>::process(FloatType x) {
        return eval(x) - x;
    }

    template<typename FloatType>
    void KneeComputer<FloatType>::interpolate() {
        tempA.store(1 / ratio.load() - 1);
        tempB.store(-threshold.load() + kneeW.load());
        tempC.store(kneeW.load() * 4);
        reductionAtKnee.store(process(threshold.load() + kneeW.load()));
    }

    template
    class KneeComputer<float>;

    template
    class KneeComputer<double>;
} // KneeComputer