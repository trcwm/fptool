/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Reader, a class that reads a source file into memory
                and provides in interface to it's characters with
                an interface that supports roll-backs.

  Author: Niels A. Moseley

*/

#include <stdio.h>
#include "reader.h"

Reader::Reader()
{
    m_curpos.line = 0;
    m_curpos.offset = 0;
    m_curpos.pos = 0;
}

Reader::~Reader()
{
}

Reader* Reader::open(const char *filename)
{
    // open file, note: must be binary,
    // otherwise number of bytes reported
    // is off due to EOL translations .. :-/
    FILE *f = fopen(filename, "rb");
    if (f == 0)
    {
        return 0;   // file cannot be opened!
    }

    Reader *reader = new Reader();

    // determine file size
    fseek(f,0,SEEK_END);
    size_t bytes = ftell(f);
    rewind(f);

    // read file into memory
    reader->m_source.resize(bytes);
    if (fread(&reader->m_source[0], 1, bytes, f) != bytes)
    {
        fclose(f);
        delete reader;
        return 0;   // number of bytes read is not equal to requested number..?
    }

    fclose(f);
    return reader;
}

bool Reader::rollback()
{
    if (!m_positions.empty())
    {
        position_info p = m_positions.top();
        m_positions.pop();
        m_curpos = p;
        return true;
    }
    return false;
}

char Reader::peek()
{
    if (m_curpos.offset < m_source.size())
    {
        return m_source[m_curpos.offset];
    }
    else
        return 0;
}

char Reader::accept()
{
    char c = peek();
    if (c != 0)
    {
        // read succesful!
        m_curpos.offset++;
        m_curpos.pos++;
        if (c == 10)
        {
            m_curpos.line++;
            m_curpos.pos = 0;
        }
        return c;
    }
    return 0;
}

void Reader::mark()
{
    m_positions.push(m_curpos);
}
