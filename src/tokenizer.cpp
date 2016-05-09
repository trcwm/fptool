/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  The tokenizer takes input from a
                source reader and produces a
                list of tokens that can be
                fed into a parser.

*/

#include "tokenizer.h"

Tokenizer::Tokenizer()
{
    // define the keywords
    // must be lower case!
    m_keywords.push_back("define");
    m_keywords.push_back("input");
    m_keywords.push_back("csd");
    m_keywords.push_back("truncate");
    m_keywords.push_back("saturate");
}

bool Tokenizer::isDigit(char c) const
{
    if ((c >= '0') && (c <= '9')) return true;
    return false;
}

bool Tokenizer::isWhitespace(char c) const
{
    if ((c == ' ') || (c == '\t')) return true;
    return false;
}

bool Tokenizer::isAlpha(char c) const
{
    if ((c >= 'A') && (c <= 'Z')) return true;
    if ((c >= 'a') && (c <= 'z')) return true;
    return false;
}

bool Tokenizer::isNumeric(char c) const
{
    if ((c >= '0') && (c <= '9')) return true;
    return false;
}

bool Tokenizer::isAlphaNumeric(char c) const
{
    if (isAlpha(c)) return true;
    if (isNumeric(c)) return true;
    return false;
}

bool Tokenizer::isAlphaNumericExtended(char c) const
{
    if (isAlpha(c)) return true;
    if (isNumeric(c)) return true;
    if (c == '_') return true;
    return false;
}

bool Tokenizer::process(Reader *r, std::vector<token_t> &result)
{
    if (r == 0)
    {
        m_lastError = std::string("Reader object was NULL!");
        return false;
    }

    tok_state_t state = S_BEGIN;
    token_t tok;
    while(state != S_DONE)
    {
        char c = r->peek();

        // check for end of file
        // if found, exit this while loop..
        if (c == 0)
        {
            state = S_DONE;
            continue;
        }

        switch(state)
        {
        case S_BEGIN:

            tok.txt.clear();
            tok.pos = r->getPos();
            tok.tokID = TOK_UNKNOWN;

            // remove whitespace
            if (isWhitespace(c))
            {
                r->accept();
                continue;
            }

            if (isAlpha(c))
            {
                tok.txt += r->accept();
                state = S_IDENT;
            }
            else if (c == 10)   // character is newline?
            {
                // emit a newline
                tok.tokID = TOK_NEWLINE;
                result.push_back(tok);
                r->accept();
            }
            else if (c == 13)   // character is a carriage return?
            {
                // ignore carriage return
                r->accept();
            }
            else if (c == ')')
            {
                tok.tokID = TOK_RPAREN;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '(')
            {
                tok.tokID = TOK_LPAREN;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '=')
            {
                tok.tokID = TOK_EQUAL;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '+')
            {
                tok.tokID = TOK_PLUS;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '-')
            {
                // is it a minus before an integer?
                // we check this by accepting
                // the character and peeking at the
                // next one to see if it is an
                // numeric character.

                r->accept();
                if (isNumeric(r->peek()))
                {
                    // it's part of an integer, or float!
                    state = S_INTEGER;
                    tok.txt = "-";
                }
                else
                {
                    // it's a single minus!
                    tok.tokID = TOK_MINUS;  // assume it's a single minus
                    result.push_back(tok);
                }
            }
            else if (c == '*')
            {
                tok.tokID = TOK_STAR;
                result.push_back(tok);
                r->accept();
            }
            else if (c == '>')
            {
                // we can't just emit a single '>'
                // because we might be looking at a '>>'
                // so, we use an additional state
                state = S_LARGER;
                r->accept();
            }
            else if (c == '<')
            {
                // we can't just emit a single '<'
                // because we might be looking at a '<<'
                // so, we use an additional state
                state = S_SMALLER;
                r->accept();
            }
            else if (c == ',')
            {
                tok.tokID = TOK_COMMA;
                result.push_back(tok);
                r->accept();
            }
            else if (c == ';')
            {
                tok.tokID = TOK_SEMICOL;
                result.push_back(tok);
                r->accept();
            }
            else if (isNumeric(c))
            {
                // we could have an integer,
                // float (with or without exponent ..)
                // first, we assume it's an integer

                tok.txt += c;
                r->accept();
                state = S_INTEGER;
            }
            else
            {
                // unknown token!
                m_lastError = std::string("Uknown token");
                return false;
            }
            break;
        case S_LARGER:
            if (c == '>')
            {
                // we have found a right-shift operator!
                tok.tokID = TOK_SHR;
                r->accept();
                result.push_back(tok);
                state = S_BEGIN;
            }
            else
            {
                // we have found a single larger than
                // and we must not accept the current
                // character!
                tok.tokID = TOK_LARGER;
                result.push_back(tok);
                state = S_BEGIN;
            }
            break;
        case S_SMALLER:
            if (c == '<')
            {
                // we have found a left-shift operator!
                tok.tokID = TOK_SHL;
                r->accept();
                result.push_back(tok);
                state = S_BEGIN;
            }
            else
            {
                // we have found a single larger than
                // and we must not accept the current
                // character!
                tok.tokID = TOK_SMALLER;
                result.push_back(tok);
                state = S_BEGIN;
            }
            break;
        case S_IDENT:   // read in keywords and identifiers
            if (isAlphaNumericExtended(c))
            {
                tok.txt += c;
                r->accept();
            }
            else
            {
                // check if it is a keyword
                bool found = false;
                for(size_t i=0; i<m_keywords.size(); i++)
                {
                    if (m_keywords[i] == tok.txt)
                    {
                        // keyword found!
                        tok.tokID = 100+i;
                        result.push_back(tok);
                        found = true;
                    }
                }
                if (!found)
                {
                    // if we end up here, it must be an
                    // identifier!
                    tok.tokID = TOK_IDENT;
                    result.push_back(tok);
                }
                state = S_BEGIN;
            }
            break;
        case S_INTEGER:
            if (isNumeric(c))
            {
                tok.txt += c;
                r->accept();
            }
            else if (c == '.')
            {
                // period means we're reading a float value
                tok.txt += '.';
                r->accept();
                state = S_FLOAT;
            }
            else
            {
                tok.tokID = TOK_INTEGER;
                result.push_back(tok);
                state = S_BEGIN;
            }
            break;
        case S_FLOAT:
            if (isNumeric(c))
            {
                tok.txt += c;
                r->accept();
            }
            else if (c == 'e')
            {
                // we're reading a float with an exponent
                tok.txt += c;
                r->accept();
                state = S_FLOAT_WITH_EXP;
            }
            else
            {
                tok.tokID = TOK_FLOAT;
                result.push_back(tok);
                state = S_BEGIN;
            }
            break;
        case S_FLOAT_WITH_EXP:
            // the exponent may be negative
            // so check for this..
            if (isNumeric(c))
            {
                // positive exponent
                tok.txt += c;
                r->accept();
                state = S_FLOAT_WITH_POSEXP;
            }
            else if (c == '-')
            {
                tok.txt += '-';
                r->accept();
                state = S_FLOAT_WITH_NEGEXP;
            }
            else
            {
                // ill-formatted floating point!
                m_lastError = std::string("Floating point has ill-formatted exponent!");
                return false;
            }
            break;
        case S_FLOAT_WITH_POSEXP:
        case S_FLOAT_WITH_NEGEXP:
            if (isNumeric(c))
            {
                tok.txt += c;
                r->accept();
            }
            else
            {
                // end of floating point
                tok.tokID = TOK_FLOAT;
                result.push_back(tok);
                state = S_BEGIN;
            }
            break;
        case S_DONE:
            break;
        default:
            tok.tokID = TOK_UNKNOWN;
            return false;
        }
    }
    return true;
}
