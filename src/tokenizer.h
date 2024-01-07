/*

    FPTOOL - a fixed-point math to VHDL generation tool

    Description:  The tokenizer takes input from a
                source reader and produces a
                list of tokens that can be
                fed into a parser.

*/

#pragma once

// ************************************
// define token ID constants
// note that keywords start
// at 100 and their ID
// is defined by their position in
// the m_keywords vector in Tokenizer
// ************************************

enum class TokenType : int
{
    UNKNOWN = 0,
    NEWLINE,
    LPAREN, 
    RPAREN,
    SEMICOL,
    PLUS,
    MINUS,   
    STAR, 
    LARGER,
    SMALLER,
    EQUAL,
    SHL,    
    SHR,  
    COMMA,
    ROL,    
    ROR,  
    SLASH,  
    INTEGER,
    FLOAT,   
    IDENT,
    END,

    // keywords
    DEFINE = 100,
    INPUT,
    CSD,
    TRUNC,
    SAT
};

#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>

#include "reader.h"

struct token_t
{
    Reader::position_info pos;      // position withing the source
    TokenType             tokID{TokenType::UNKNOWN}; // token identifier
    std::string           txt;      // identifier, keyword or number string
};

class Tokenizer
{
public:
  Tokenizer();

  /** read the source and produce a list of tokens.
      returns false if an error occurred.
  */
  bool process(Reader *r, std::vector<token_t> &result);

  /** returns the last error in string form */
  std::string getErrorString() const
  {
    return m_lastError;
  }

  void dumpTokens(std::ostream &stream, const std::vector<token_t> &tokens);

protected:
  static bool isDigit(char c);
  static bool isWhitespace(char c);
  static bool isAlpha(char c);
  static bool isNumeric(char c);
  static bool isAlphaNumeric(char c);
  static bool isAlphaNumericExtended(char c);

  enum tok_state_t {S_BEGIN,
                    S_IDENT,
                    S_INTEGER,
                    S_FLOAT,
                    S_FLOAT_WITH_EXP,
                    S_FLOAT_WITH_POSEXP,
                    S_FLOAT_WITH_NEGEXP,
                    S_LARGER,
                    S_SMALLER,
                    S_ROL,
                    S_ROR,
                    S_COMMENT,
                    S_DONE};

  std::string               m_lastError;
  std::vector<std::string>  m_keywords;
};
