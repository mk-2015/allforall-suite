#include<dot3/str3.hpp>
#include<utility>
#include<ostream>
#include<cstddef>

namespace dotthree::Strings 
{
    BetterString::BetterString(std::string InputString) : InternalString(std::move(InputString)) {
		length = InternalString.length();
	}
		
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

	std::string BetterString::ins() {
		return InternalString;
	}

	/*
	BetterString& BetterString::operator=(const std::string& other) {
		InternalString = other; 
		length = other.length();

		return *this;
	}*/

	int BetterString::toint() {
		try {
			return std::stoi(InternalString);
		} catch (const std::invalid_argument& e) {
			return 0;
		} catch (const std::out_of_range& e) {
			return 0;
		}
	}

	unsigned int BetterString::touint() {
		try {
			return static_cast<unsigned int>(std::stoul(InternalString));
		} catch (const std::invalid_argument& e) {
			return 0;
		} catch (const std::out_of_range& e) {
			return 0;
		}
	}

	BetterString& BetterString::operator=(const BetterString& other) {
		if (this != &other) {
			InternalString = other.InternalString;
			length = other.length;
		}
		else if (this == &other) {
			InternalString = other.InternalString;
			length = other.length;
		}

		return *this;
	}

	dotthree::Strings::BetterString cut(std::string s) {
        return dotthree::Strings::BetterString(s);
    }

	std::string uncut(dotthree::Strings::BetterString s) {
        return s.ins();
    }
	
}