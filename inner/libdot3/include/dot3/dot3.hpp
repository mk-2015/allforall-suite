/*
 libdot3 - dot3/dot3.hpp
 Under the GNU GPL V3.0 LICENSE.
 
 Regex Matchig library
*/

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <stack>

namespace dotthree::Regex {

enum StateType {
    MATCH_CHAR,
    MATCH_ANY, 
    MATCH_CLASS,
    SPLIT,
    MATCH_END
};

struct State {
    StateType type;
    char literal = 0;
    std::set<char> charClass;
    
    std::shared_ptr<State> out = nullptr;
    std::shared_ptr<State> out1 = nullptr;  
};

struct Fragment {
    std::shared_ptr<State> start;
    std::vector<std::shared_ptr<State>*> outArrows;
};

class regex {
public:
    explicit regex(std::string RegexString);
    bool Parse(std::string StringToParse);

private:
    std::string regexString;
    std::shared_ptr<State> nfaStart;

    std::string InfixToPostfix(const std::string& infix);
    std::shared_ptr<State> BuildNFA(const std::string& postfix);
    void AddState(std::shared_ptr<State> s, std::set<std::shared_ptr<State>>& currentStates);
};

}

/*
#ifdef __cplusplus
extern "C" 
{
    REGEX_API void* NewRegexHandler(const char* regexString);
    REGEX_API void CloseRegexHandler(void* handle);
	REGEX_API bool ParseRegexString(void* handle, const char* StringToParse);
}
#endif
*/