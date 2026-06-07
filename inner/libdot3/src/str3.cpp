#include<dot3/str3.hpp>

namespace dotthree::Strings 
{
    BetterString::BetterString(std::string InputString)
		: InternalString(std::move(InputString)) {}
		
    char& BetterString::operator[](size_t index) {
        return InternalString[index];
    }

    const char& BetterString::operator[](size_t index) const {
        return InternalString[index];
    }
	
	bool BetterString::operator==(const BetterString& other) const {
		return this->InternalString == other.InternalString;
	}
	
	BetterString& BetterString::operator+=(const BetterString& other) {
		this->InternalString += other.InternalString;
		return *this;
	}
	
	BetterString BetterString::operator+(const BetterString& other) const {
		std::string combined = this->InternalString + other.InternalString;
		
		return BetterString(std::move(combined));
	}


	std::ostream& operator<<(std::ostream& os, const BetterString& str) {
		os << str.InternalString;
		return os;
	}

}