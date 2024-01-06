/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  Reader, a class that reads a source file into memory
                and provides in interface to it's characters with
                an interface that supports roll-backs.

  Author: Niels A. Moseley

*/

#ifndef reader_h
#define reader_h

#include <stack>
#include <vector>

/** The reader object reads a source file into memory and provides
    an interface to the characters that supports roll-backs.
*/

class Reader
{
public:
  virtual ~Reader();

  struct position_info
  {
    size_t  offset{0};     // offset into m_source
    size_t  line{0};       // the line number
    size_t  pos{0};        // the position within the line
  };

  /** Create a reader object by opening a file.
      NULL is returned when an error occured. */
  static Reader* open(const char *filename);

  /** Rollback the read pointer to the last marked position.
      When succesfull, the marked position is removed from
      the stack.
      Returns false if there are no markers left.
    */
  bool rollback();

  /** Mark the current read position so we can roll back to
      it later */
  void mark();

  /** Get the charater at the current read position.
        The read position is not advanced.
        When there are no characters to read,
        it returns 0.
    */
  char peek();

  /** Read the character at the current read position.
      The read position is advanced one character.
      When there are no characters to read,
      it returns 0.
    */
  char accept();

  /** Get the current read position */
  position_info getPos() const
  {
    return m_curpos;
  }

protected:
  /** Hide the constructor so the user can only get
      a Reader object by using 'open'.
    */
  Reader();

  std::vector<char>   m_source;               // the source code
  std::stack<position_info>  m_positions;     // a stack to hold roll-back positions.
  position_info   m_curpos;                   // the current read position.
};

#endif
