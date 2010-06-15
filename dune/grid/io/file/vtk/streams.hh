// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_GRID_IO_FILE_VTK_STREAMS_HH
#define DUNE_GRID_IO_FILE_VTK_STREAMS_HH

#include <ostream>

#include <dune/grid/io/file/vtk/b64enc.hh>

namespace Dune {

  //! class to base64 encode a stream of data
  class Base64Stream {
    std::ostream& s;
    b64chunk chunk;
    char obuf[4];

  public:
    //! Construct a Base64Stream
    /**
     * \param s_ The stream the resulting base64-encoded text will be written
     *           to.
     */
    Base64Stream(std::ostream& s_)
      : s(s_)
    {
      // reset chunk
      chunk.txt.read(0,0);
    }

    //! encode a data item
    /**
     * The result will be written to the stream, eventually.  This method may
     * be called multiple times in a row.  After this method has been called,
     * noone else may write to the undelying stream until flush() has been
     * called or this writer object has been destroyed.
     */
    template <class X>
    void write(X & data)
    {
      char* p = reinterpret_cast<char*>(&data);
      for (size_t len = sizeof(X); len > 0; len--,p++)
      {
        chunk.txt.put(*p);
        if (chunk.txt.size == 3)
        {
          chunk.data.write(obuf);
          s.write(obuf,4);
        }
      }
    }

    //! flush the current unwritten data to the stream.
    /**
     * If the size of the received input is not a multiple of three bytes, an
     * end-marker will be written.
     *
     * Calling this function a second time without calling b64enc() or calling
     * it right after construction has no effect.
     */
    void flush()
    {
      if (chunk.txt.size > 0)
      {
        chunk.data.write(obuf);
        s.write(obuf,4);
      }
    }

    //! destroy the object
    /**
     * Calls flush()
     */
    ~Base64Stream() {
      flush();
    }
  };

} // namespace Dune

#endif // DUNE_GRID_IO_FILE_VTK_STREAMS_HH