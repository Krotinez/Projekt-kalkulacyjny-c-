#include "PowerVisual.h"
#include <algorithm>
#include <cmath>
#include <functional>

namespace {
char32_t bracketGlyph(char32_t c) { switch(c) { case U'\uE130': return U')'; case U'\uE131': return U']'; case U'\uE132': return U'}'; case U'\uE133': case U'<': case U'>': return U'|'; default: return c; } }
bool ghost(char32_t c) {return c>=U'\uE130' && c<=U'\uE133';}

bool opening(char32_t c) { return c==U'(' || c==U'[' || c==U'{'; }
bool closing(char32_t c) { return c==U')' || c==U']' || c==U'}' || ghost(c); }
bool numeric(char32_t c) { return (c>=U'0' && c<=U'9') || c==U',' || c==U'.'; }
}
PowerVisual makePowerVisual(const std::string& line,const sf::Font& font,unsigned normalSize) {
    const auto s=sf::String::fromUtf8(line.begin(),line.end());
    const std::size_t n=s.getSize();
    PowerVisual v;
    v.x.assign(n+1,0.f);v.rise.assign(n+1,0.f);v.size.assign(n+1,normalSize);
    std::vector<int> depth(n,0);
    // Parse exponent ranges, including an empty exponent and nested power.
    // Parenthesized exponents may contain arbitrary expressions.
    std::function<void(std::size_t,std::size_t,int)> scan;
    scan=[&](std::size_t first,std::size_t last,int level) {
        for(std::size_t i=first;i<last;++i) {
            if(s[i]!=U'^') continue;
            const int next=std::min(2,level+1);
            const std::size_t begin=i+1;
            std::size_t end=begin;
            if(begin<last && opening(s[begin])) {
                int nesting=0;
                for(;end<last;++end) {
                    if(opening(s[end])) ++nesting;
                    else if(closing(s[end]) && --nesting==0) {++end;break;}
                }
            } else {
                if(end<last && s[end]==U'-') ++end;
                while(end<last && numeric(s[end])) ++end;
                if(end<last && (s[end]==U'%' || s[end]==U'‰' || s[end]==U'!')) ++end;
                // Unbracketed nested power: 2^2^3, parsed right-associatively.
                if(end<last && s[end]==U'^') {
                    std::size_t j=end+1;
                    if(j<last && opening(s[j])) {
                        int nesting=0;
                        for(;j<last;++j) {
                            if(opening(s[j])) ++nesting;
                            else if(closing(s[j]) && --nesting==0) {++j;break;}
                        }
                    } else {
                        while(j<last && numeric(s[j])) ++j;
                        if(j<last && (s[j]==U'%' || s[j]==U'‰' || s[j]==U'!')) ++j;
                    }
                    end=j;
                }
            }
            v.exponents.push_back({begin,end,next});
            // The caret before ^ belongs to the base, not to its exponent.
            depth[i]=std::max(depth[i],level);
            for(std::size_t j=begin;j<end;++j) depth[j]=std::max(depth[j],next);
            if(end>begin) scan(begin,end,next);
            i=end>i?end-1:i;
        }
    };
    scan(0,n,0);
    float advance=0.f;
    for(std::size_t i=0;i<n;++i) {
        v.x[i]=advance;
        const int level=depth[i];
        v.size[i]=level==0?normalSize:(level==1?std::max(12u,normalSize*72/100):std::max(10u,normalSize*54/100));
        v.rise[i]=level==0?0.f:(level==1?12.f:22.f);
        if(s[i]==U'^') continue;
        sf::Text glyph(font,sf::String(bracketGlyph(s[i])),v.size[i]);
        advance+=font.getGlyph(bracketGlyph(s[i]),v.size[i],false).advance;
    }
    v.x[n]=advance;v.width=advance;
    // Oddzielna geometria kursora: granica konca indeksu moze byc
    // jednoczesnie poczatkiem operatora na poziomie podstawowym.
    v.caretRise = v.rise;
    // Cursor after '^' belongs to the newly opened exponent, including empty ones.
    for(const auto& exponent:v.exponents) {
        if(exponent.start<=n) {
            v.caretRise[exponent.start]=exponent.level==2?22.f:12.f;
            v.size[exponent.start]=exponent.level==2?std::max(10u,normalSize*54/100):std::max(12u,normalSize*72/100);
        }
    }
    // Pozycja tuz za ostatnim znakiem wykladnika nalezy jeszcze do
    // indeksu gornego. Bez tego kursor opadal do poziomu podstawy
    // przed wykonaniem dodatkowego kroku wyjscia z potegi.
    // Dla zagniezdzen wybieramy najglebszy indeks konczacy sie tutaj.
    for (const auto& exponent : v.exponents) {
        if (exponent.end > exponent.start && exponent.end <= n) {
            const float exponentRise = exponent.level == 2 ? 22.f : 12.f;
            if (v.caretRise[exponent.end] < exponentRise)
                v.caretRise[exponent.end] = exponentRise;
        }
    }
    return v;
}
void drawPowerVisual(sf::RenderWindow& window,const std::string& line,
    const sf::Font& font,unsigned normalSize,sf::Vector2f origin,sf::Color color) {
    const auto s=sf::String::fromUtf8(line.begin(),line.end());
    const auto v=makePowerVisual(line,font,normalSize);
    for(std::size_t i=0;i<s.getSize();++i) {
        if(s[i]==U'^') continue;
        sf::Text glyph(font,sf::String(bracketGlyph(s[i])),v.size[i]);
        glyph.setFillColor(ghost(s[i]) ? sf::Color(145,145,145,190) : color);
        glyph.setPosition({origin.x+v.x[i],origin.y-v.rise[i]});
        window.draw(glyph);
    }
}
