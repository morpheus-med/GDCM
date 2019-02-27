#include <streambuf>
#include <cstring>
#include <vector>
#include <istream>

/**
 gdcmReader does seeking when decoding the headers so we need to buffer the input stream
 can seek back at least half the buffer size, fails when seeking outside of the buffer
 based on https://stackoverflow.com/questions/44711735/someway-to-buffer-or-wrap-cin-so-i-can-use-tellg-seekg
*/
class fixed_istream_buffer : public std::streambuf {
    std::istream& stream;
    const unsigned size;
    const unsigned half;
    std::vector<char> buffer;
    std::streamoff base;

public:
    fixed_istream_buffer(std::istream& stream, unsigned size)
        : stream(stream), size(size), half(size / 2), buffer(size), base() {
        std::streamsize read = stream.read(buffer.data(), size).gcount();
        setg(buffer.data(), buffer.data(), buffer.data() + read);
    }

    int underflow() override {
        if (gptr() == buffer.data() + size) {
            std::memmove(eback(), eback() + half, half);
            base += half;
            std::streamsize read = stream.read(eback() + half, half).gcount();
            setg(eback(), eback() + half, eback() + half + read);
        }
        return gptr() != egptr()
            ? traits_type::to_int_type(*gptr())
            : traits_type::eof();
    }

    // this could support limited std::ios_base::beg
    std::streampos seekoff(off_type offset, std::ios_base::seekdir way, std::ios_base::openmode which) override {
        if (gptr() - eback() < -offset
            || egptr() - gptr() < offset
            || way != std::ios_base::cur
            || !(which & std::ios_base::in)) {
            return pos_type(off_type(-1));
        }
        gbump(offset);
        return pos_type(base + (gptr() - eback()));
    }

    std::streampos seekpos(pos_type pos, std::ios_base::openmode which) override {
        if (off_type(pos) < base
            || base + (egptr() - eback()) < off_type(pos)
            || !(which & std::ios_base::in)) {
           return pos_type(off_type(-1));
        }
        setg(eback(), eback() + (off_type(pos) - base), egptr());
        return pos_type(base + (gptr() - eback()));
    }
};
