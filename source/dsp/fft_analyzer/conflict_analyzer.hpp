// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_CONFLICT_ANALYZER_HPP
#define ZLEqualizer_CONFLICT_ANALYZER_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "multiple_fft_analyzer.hpp"

namespace zlFFT {
    /**
     * a conflict fft analyzer
     * @tparam FloatType
     */
    template<typename FloatType>
    class ConflictAnalyzer final : private juce::Thread, juce::AsyncUpdater {
    public:
        static constexpr size_t pointNum = 400;
        explicit ConflictAnalyzer();

        ~ConflictAnalyzer() override;

        void prepare(const juce::dsp::ProcessSpec &spec);

        void start() {
            toReset.store(true);
            startThread(juce::Thread::Priority::low);
        }

        void stop() {
            stopThread(-1);
        }

        void setON(bool x);

        bool getON() const { return isON.load(); }

        void setStrength(const FloatType x) { strength.store(x); }

        void setConflictScale(const FloatType x) { conflictScale.store(x); }

        void pushMainBuffer(juce::AudioBuffer<FloatType> &buffer);

        void pushRefBuffer(juce::AudioBuffer<FloatType> &buffer);

        void process();

        void setLeftRight(const float left, const float right) {
            x1.store(left);
            x2.store(right);
        }

        void updateGradient(juce::ColourGradient &gradient);

        MultipleFFTAnalyzer<FloatType, 2, pointNum> &getSyncFFT() { return syncAnalyzer; }

    private:
        MultipleFFTAnalyzer<FloatType, 2, pointNum> syncAnalyzer;
        juce::AudioBuffer<FloatType> mainBuffer, refBuffer;
        std::array<float, pointNum> mainDB{}, refDB{};
        std::atomic<FloatType> strength{.375f}, conflictScale{1.f};
        std::atomic<bool> isON{false}, isConflictReady{false}, toReset{false};
        bool currentIsON{false};

        std::atomic<float> x1{0.f}, x2{1.f};
        std::array<float, pointNum / 4> conflicts{};
        std::array<std::atomic<float>, pointNum / 4> conflictsP{};

        const juce::Colour gColour = juce::Colours::red;

        void run() override;

        void handleAsyncUpdate() override;
    };
} // zlFFT

#endif //ZLEqualizer_CONFLICT_ANALYZER_HPP
