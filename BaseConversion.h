#pragma once
#include <SFML/System/String.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>

// Converts numeric literals only. Structural markers, operators and field boundaries
// are preserved; cursor/selection offsets are Unicode code-point offsets.
namespace BaseConversion {
inline int digit(char32_t c) {
    if(c>=U'0'&&c<=U'9') return int(c-U'0');
    if(c>=U'A'&&c<=U'F') return int(c-U'A')+10;
    if(c>=U'a'&&c<=U'f') return int(c-U'a')+10;
    return -1;
}
inline char32_t symbol(int d) { return U"0123456789ABCDEF"[d]; }
inline bool isNumberChar(char32_t c,int base) {
    const int d=digit(c); return d>=0&&d<base;
}
inline sf::String encode(long double value,int base,bool comma) {
    sf::String result;
    if(!std::isfinite(value) || value<0) return result;
    long double integer=std::floor(value);
    sf::String digits;
    // Guard against runaway conversions of extremely large expressions.
    for(int i=0;i<512;++i) {
        const long double quotient=std::floor(integer/base);
        int remainder=int(std::round(integer-quotient*base));
        remainder=std::max(0,std::min(base-1,remainder));
        digits.insert(0,sf::String(symbol(remainder)));
        integer=quotient;
        if(integer<1) break;
    }
    result+=digits;
    long double fraction=value-std::floor(value);
    if(fraction>1e-14L) {
        result+=comma?U',':U'.';
        for(int i=0;i<16&&fraction>1e-14L;++i) {
            fraction*=base;
            const int d=std::max(0,std::min(base-1,int(std::floor(fraction+1e-14L))));
            result+=symbol(d);
            fraction-=d;
        }
    }
    return result;
}
inline bool convert(const std::string& input,int from,int to,std::string& output,
                    std::size_t& cursor,std::size_t& anchor) {
    if(from==to) {output=input;return true;}
    const sf::String src=sf::String::fromUtf8(input.begin(),input.end());
    sf::String dst;
    std::size_t newCursor=0,newAnchor=0;
    auto mapRange=[&](std::size_t begin,std::size_t end,std::size_t outBegin,std::size_t outEnd) {
        auto mapOne=[&](std::size_t position) {
            if(position<=begin) return outBegin;
            if(position>=end) return outEnd;
            return outBegin+(position-begin)*(outEnd-outBegin)/(end-begin);
        };
        if(cursor>=begin&&cursor<=end) newCursor=mapOne(cursor);
        if(anchor>=begin&&anchor<=end) newAnchor=mapOne(anchor);
    };
    std::size_t i=0;
    while(i<src.getSize()) {
        const std::size_t start=i, outStart=dst.getSize();
        // Never interpret letters inside textual function names as HEX digits.
        if(i+4<=src.getSize() && src.substring(i,4)==sf::String("log(")) {
            dst+=src.substring(i,4);i+=4;
        } else if(i+5<=src.getSize() && src.substring(i,5)==sf::String("root(")) {
            dst+=src.substring(i,5);i+=5;
        } else if(isNumberChar(src[i],from) || ((src[i]==U','||src[i]==U'.') && i+1<src.getSize() && isNumberChar(src[i+1],from))) {
            long double value=0,unit=1;
            bool decimal=false, comma=false;
            while(i<src.getSize()) {
                const int d=digit(src[i]);
                if(d>=0&&d<from) {
                    if(!decimal) value=value*from+d;
                    else {unit/=from;value+=d*unit;}
                    ++i;
                } else if(!decimal&&(src[i]==U','||src[i]==U'.') && i+1<src.getSize()&&isNumberChar(src[i+1],from)) {
                    decimal=true;comma=src[i]==U',';++i;
                } else break;
            }
            const auto number=encode(value,to,comma);
            if(number.isEmpty()) return false;
            dst+=number;
        } else { dst+=src[i];++i; }
        mapRange(start,i,outStart,dst.getSize());
    }
    auto bytes=dst.toUtf8();output=std::string(bytes.begin(),bytes.end());
    cursor=std::min(newCursor,dst.getSize());anchor=std::min(newAnchor,dst.getSize());
    return true;
}
}
