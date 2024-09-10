// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLFILTER_IDEAL_SINGLE_FILTER_HPP
#define ZLFILTER_IDEAL_SINGLE_FILTER_HPP

#include "ideal_base.hpp"
#include "coeff/ideal_coeff.hpp"
#include "../filter_design/filter_design.hpp"

namespace zlFilter {
    template<typename FloatType, size_t FilterSize>
    class Ideal {
    public:
        Ideal() = default;

        void prepare(const double sampleRate) {
            fs.store(sampleRate);
            toUpdatePara.store(true);
        }

        void setFreq(const FloatType x) {
            freq.store(x);
            toUpdatePara.store(true);
        }

        void setGain(const FloatType x) {
            gain.store(x);
            toUpdatePara.store(true);
        }

        void setQ(const FloatType x) {
            q.store(x);
            toUpdatePara.store(true);
        }

        void setFilterType(const FilterType x) {
            filterType.store(x);
            toUpdatePara.store(true);
        }

        void setOrder(const size_t x) {
            order.store(x);
            toUpdatePara.store(true);
        }

        void prepareDBSize(size_t x) {
            dBs.resize(x);
            gains.resize(x);
        }

        bool getToUpdatePara() const { return toUpdatePara.load(); }

        void updateMagnidue(const std::vector<FloatType> &ws) {
            if (toUpdatePara.exchange(false)) {
                updateParas();
            }
            std::fill(gains.begin(), gains.end(), FloatType(1));
            for (size_t i = 0; i < filterNum.load(); ++i) {
                filters[i].updateFromIdeal(coeffs[i]);
                filters[i].updateMagnidue(ws, gains);
            }
            std::transform(gains.begin(), gains.end(), dBs.begin(),
                           [](auto &g) {
                               return g > 0 ? std::log10(g) * FloatType(20) : FloatType(-480);
                           });
        }

        void addDBs(std::vector<FloatType> &x) {
            std::transform(x.begin(), x.end(), dBs.begin(), x.begin(),
                           [](auto &c1, auto &c2) { return c1 + c2; });
        }

    private:
        std::array<IdealBase<FloatType>, FilterSize> filters{};
        std::array<std::array<double, 6>, FilterSize> coeffs{};
        std::atomic<bool> toUpdatePara{true};
        std::atomic<size_t> filterNum{1}, order{2};
        std::atomic<double> freq{1000.0}, gain{0.0}, q{0.707};
        std::atomic<double> fs{48000.0};
        std::atomic<FilterType> filterType = FilterType::peak;
        std::vector<FloatType> dBs{}, gains{};

        void updateParas() {
            filterNum.store(updateIIRCoeffs(updateIIRCoeffs(filterType.load(), order.load(),
                                                            freq.load(), fs.load(),
                                                            gain.load(), q.load(), coeffs)));
        }

        static size_t updateIIRCoeffs(const FilterType filterType, const size_t n,
                                      const double f, const double fs, const double g0, const double q0,
                                      std::array<std::array<double, 6>, 16> &coeffs) {
            return FilterDesign::updateCoeffs<16,
                IdealCoeff::get1LowShelf, IdealCoeff::get1HighShelf, IdealCoeff::get1TiltShelf,
                IdealCoeff::get1LowPass, IdealCoeff::get1HighPass,
                IdealCoeff::get2Peak,
                IdealCoeff::get2LowShelf, IdealCoeff::get2HighShelf, IdealCoeff::get2TiltShelf,
                IdealCoeff::get2LowPass, IdealCoeff::get2HighPass,
                IdealCoeff::get2BandPass, IdealCoeff::get2Notch>(
                filterType, n, f, fs, g0, q0, coeffs);
        }
    };
}

#endif //ZLFILTER_IDEAL_SINGLE_FILTER_HPP
