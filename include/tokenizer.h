/*

    FPTOOL - a fixed-point math to VHDL generation tool

    Description:  The tokenizer takes input from a
                source reader and produces a
                list of tokens that can be
                fed into a parser.

*/

#ifndef tokenizer_h
#define tokenizer_h

// ************************************
// define token ID constants
// note that keywords start
// at 100 and their ID
// is defined by their position in
// the m_keywords vector in Tokenizer
// ************************************

#define TOK_UNKNOWN 0
#define TOK_NEWLINE 1
#define TOK_LPAREN  2
#define TOK_RPAREN  3
#define TOK_SEMICOL 4
#define TOK_PLUS    5
#define TOK_MINUS   6
#define TOK_STAR    7
#define TOK_LARGER  8
#define TOK_SMALLER 9
#define TOK_EQUAL   10
#define TOK_SHL     11
#define TOK_SHR     12
#define TOK_COMMA   13
#define TOK_ROL     14
#define TOK_ROR     15

#define TOK_INTEGER 30
#define TOK_FLOAT   31
#define TOK_IDENT   32


#include <vector>
#include <string>
#include <stdint.h>

#include "reader.h"

struct token_t
{
  Reader::position_info pos;      // position withing the source
  uint32_t              tokID;    // token identifier
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

protected:
  bool isDigit(char c) const;
  bool isWhitespace(char c) const;
  bool isAlpha(char c) const;
  bool isNumeric(char c) const;
  bool isAlphaNumeric(char c) const;
  bool isAlphaNumericExtended(char c) const;

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


#endif
