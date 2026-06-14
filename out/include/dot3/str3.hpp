#pragma once
#include<string>
#include<cstdint>

namespace dotthree::Strings 
{

class BetterString {
private:
    std::string InternalString;

public:
    uint64_t length;

    explicit BetterString(std::string InputString);

    std::string ins();
	char& operator[](size_t index);
    const char& operator[](size_t index) const;
	bool operator==(const BetterString& other) const;
	BetterString& operator+=(const BetterString& other);
	BetterString operator+(const BetterString& other) const;
    friend std::ostream& operator<<(std::ostream& os, const BetterString& str);
    BetterString& operator=(const BetterString& other);
    // BetterString& BetterString::operator=(const std::string& other);

    int toint();
    unsigned int touint();
    
};
	
dotthree::Strings::BetterString cut(std::string s);
std::string uncut(dotthree::Strings::BetterString s);
}