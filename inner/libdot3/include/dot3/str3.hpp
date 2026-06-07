#pragma once

namespace dotthree::Strings 
{

class BetterString {
private:
    std::string InternalString;

public:
    explicit BetterString(std::string InputString);
    
	char& operator[](size_t index);
    const char& operator[](size_t index) const;
	bool operator==(const BetterString& other) const;
	BetterString& operator+=(const BetterString& other);
	BetterString operator+(const BetterString& other) const;
    friend std::ostream& operator<<(std::ostream& os, const BetterString& str);
};
	
}