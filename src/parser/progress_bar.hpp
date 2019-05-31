//
//  progress_bar.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/28/17.
//

#ifndef progress_bar_hpp
#define progress_bar_hpp

#include <cstdint>
#include <stdio.h>
#include <iostream>
#include <cmath>

template<typename UpdateFunc>
class ProgressBar {
    uint64_t total;
    uint64_t percentageMarker;
    UpdateFunc updateFunc;
    
public:
    ProgressBar(uint64_t total_, UpdateFunc updateFunc_) : total(total_), updateFunc(updateFunc_) {
        auto percentage = static_cast<double>(total) / 1000.0;
        percentageMarker = static_cast<uint64_t>(std::ceil(percentage));
        std::cout.setf(std::ios::fixed,std::ios::floatfield);
        std::cout.precision(2);
    }

    ProgressBar(const ProgressBar &) = default;
    ProgressBar(ProgressBar &&) = default;
    
    ~ProgressBar() {
        std::cout << "\n";
    }
    
    template <typename... Args>
    void update(uint64_t currentCount, Args... args) {
        if (currentCount % 10000 == 0) {
            auto percentDone = (static_cast<double>(currentCount) / static_cast<double>(total)) * 100;
            std::cout << "\r" << percentDone << "% done";
            updateFunc(args...);
            std::cout << std::flush;;
        }
    }
};

template<class UpdateFunc>
ProgressBar<UpdateFunc> makeProgressBar(uint64_t total, UpdateFunc updateFunc) {
    return {total, updateFunc};
}

#endif /* progress_bar_hpp */
