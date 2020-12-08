#pragma once
#include <cmath>

namespace utilities {

template<typename Integer, typename Float>
Integer getIntegerPart(Float in) {
    return Integer(std::floor(in));
}

template<typename Integer, typename Float>
Float getFractionalPart(Float in) {
    return (in - Float(getIntegerPart<Integer,Float>(in)));
}

template<typename Integer, typename Float>
bool isInteger(Float in) {
    return getFractionalPart<Integer,Float>(in) == Float(0);
}

}