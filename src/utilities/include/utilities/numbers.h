#pragma once

namespace utilities {

template<typename Integer, typename Float>
Integer getIntegerPart(Float in) {
    return Integer(in);
}

template<typename Integer, typename Float>
Float getFractionalPart(Float in) {
    return in - Float(getIntegerPart(in));
}

}