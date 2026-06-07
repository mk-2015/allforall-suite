#include <dot3/dot3.hpp>
#include <dot3/dot3.h>

namespace dothree::Regex {

regex::regex(std::string RegexString) : regexString(std::move(RegexString)) {
    std::string explicitConcat = "";
    
    for (size_t i = 0; i < regexString.length(); ++i) {
        char c1 = regexString[i];
        explicitConcat += c1;
        if (i + 1 < regexString.length()) {
            char c2 = regexString[i + 1];
            if (c1 != '(' && c1 != '|' && c2 != ')' && c2 != '|' && c2 != '*' && c2 != '+' && c2 != '?') {
                explicitConcat += '\x08';
            }
        }
    }

    std::string postfix = InfixToPostfix(explicitConcat);
    
    nfaStart = BuildNFA(postfix);
}

std::string regex::InfixToPostfix(const std::string& infix) {
    std::string postfix = "";
    std::stack<char> operators;
    
    auto precedence = [](char c) {
        if (c == '*' || c == '+' || c == '?') return 3;
        if (c == '\x08') return 2;
        if (c == '|') return 1;
        return 0;
    };

    for (size_t i = 0; i < infix.length(); ++i) {
        char c = infix[i];
        
        if (c == '[') {
            postfix += c;
            while (i < infix.length() && infix[i] != ']') {
                postfix += infix[++i];
            }
            continue;
        }

        if (c == '(') {
            operators.push(c);
        } else if (c == ')') {
            while (!operators.empty() && operators.top() != '(') {
                postfix += operators.top();
                operators.pop();
            }
            if (!operators.empty()) operators.pop(); 
        } else if (precedence(c) > 0) {
            while (!operators.empty() && precedence(operators.top()) >= precedence(c)) {
                postfix += operators.top();
                operators.pop();
            }
            operators.push(c);
        } else {
            postfix += c;
        }
    }
    while (!operators.empty()) {
        postfix += operators.top();
        operators.pop();
    }
    return postfix;
}

std::shared_ptr<State> regex::BuildNFA(const std::string& postfix) {
    std::stack<Fragment> fragStack;

    auto connect = [](std::vector<std::shared_ptr<State>*>& arrows, std::shared_ptr<State> dst) {
        for (auto* arrow : arrows) {
            *arrow = dst;
        }
    };

    for (size_t i = 0; i < postfix.length(); ++i) {
        char c = postfix[i];

        if (c == '[') {
            auto s = std::make_shared<State>();
            s->type = MATCH_CLASS;
            i++;
            while (postfix[i] != ']') {
                s->charClass.insert(postfix[i++]);
            }
            fragStack.push(Fragment{s, {&s->out}});
        } 
        else if (c == '.') {
            auto s = std::make_shared<State>();
            s->type = MATCH_ANY;
            fragStack.push(Fragment{s, {&s->out}});
        } 
        else if (c == '\x08') {
            auto e2 = fragStack.top(); fragStack.pop();
            auto e1 = fragStack.top(); fragStack.pop();
            connect(e1.outArrows, e2.start);
            fragStack.push(Fragment{e1.start, e2.outArrows});
        } 
        else if (c == '|') {
            auto e2 = fragStack.top(); fragStack.pop();
            auto e1 = fragStack.top(); fragStack.pop();
            auto s = std::make_shared<State>();
            s->type = SPLIT;
            s->out = e1.start;
            s->out1 = e2.start;
            
            std::vector<std::shared_ptr<State>*> mergedOut = e1.outArrows;
            mergedOut.insert(mergedOut.end(), e2.outArrows.begin(), e2.outArrows.end());
            fragStack.push(Fragment{s, mergedOut});
        } 
        else if (c == '*') {
            auto e = fragStack.top(); fragStack.pop();
            auto s = std::make_shared<State>();
            s->type = SPLIT;
            s->out = e.start;
            connect(e.outArrows, s);
            fragStack.push(Fragment{s, {&s->out1}});
        } 
        else if (c == '+') {
            auto e = fragStack.top(); fragStack.pop();
            auto s = std::make_shared<State>();
            s->type = SPLIT;
            s->out = e.start;
            connect(e.outArrows, s);
            fragStack.push(Fragment{e.start, {&s->out1}});
        } 
        else if (c == '?') {
            auto e = fragStack.top(); fragStack.pop();
            auto s = std::make_shared<State>();
            s->type = SPLIT;
            s->out = e.start;
            
            std::vector<std::shared_ptr<State>*> mergedOut = {&s->out1};
            mergedOut.insert(mergedOut.end(), e.outArrows.begin(), e.outArrows.end());
            fragStack.push(Fragment{s, mergedOut});
        } 
        else {
            auto s = std::make_shared<State>();
            s->type = MATCH_CHAR;
            s->literal = c;
            fragStack.push(Fragment{s, {&s->out}});
        }
    }

    auto matchState = std::make_shared<State>();
    matchState->type = MATCH_END;
    
    if (fragStack.empty()) return matchState;
    
    auto finalFrag = fragStack.top();
    connect(finalFrag.outArrows, matchState);
    return finalFrag.start;
}

void regex::AddState(std::shared_ptr<State> s, std::set<std::shared_ptr<State>>& currentStates) {
    if (!s || currentStates.count(s)) return;
    
    if (s->type == SPLIT) {
        // Instantly follow free paths (epsilon-transitions)
        AddState(s->out, currentStates);
        AddState(s->out1, currentStates);
    } else {
        currentStates.insert(s);
    }
}

bool regex::Parse(std::string StringToParse) {
    std::set<std::shared_ptr<State>> currentStates;
    AddState(nfaStart, currentStates);

    for (char c : StringToParse) {
        std::set<std::shared_ptr<State>> nextStates;
        
        for (const auto& state : currentStates) {
            if (state->type == MATCH_CHAR && state->literal == c) {
                AddState(state->out, nextStates);
            } 
            else if (state->type == MATCH_ANY) {
                AddState(state->out, nextStates);
            } 
            else if (state->type == MATCH_CLASS && state->charClass.count(c)) {
                AddState(state->out, nextStates);
            }
        }
        currentStates = std::move(nextStates);
    }

    for (const auto& state : currentStates) {
        if (state->type == MATCH_END) return true;
    }
    return false;
}
}

#include "regex.h"
#include <cstring>
#include <new>

extern "C" 
{
    void* NewRegexHandler(const char* regexString)
    {
        if (!regexString) return nullptr;
        
        return static_cast<void*>(new(std::nothrow) Regex::regex(std::string(regexString)));
    }

    void CloseRegexHandler(void* handler)
    {
        if (handler) 
        {
            delete static_cast<Regex::regex*>(handler);
        }
    }

    bool ParseRegexString(void* handle, const char* StringToParse)
    {
        if (!handle || !StringToParse) return false;
        
        return static_cast<Regex::regex*>(handle)->Parse(std::string(StringToParse));
    }
}
