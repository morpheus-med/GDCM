#include <streambuf>
#include <cstring>
#include <vector>
#include <istream>

/**
 gdcmReader does seeking when decoding the headers so we need to buffer the input stream
 can seek back at least half the buffer size, fails when seeking outside of the buffer
 may make a blocking read on construction
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
            char* half_buffer = eback() + half;
            std::memmove(eback(), half_buffer, half);
            std::streamsize read = stream.read(half_buffer, half).gcount();
            setg(eback(), half_buffer, half_buffer + read);
            base += half;
        }
        return has_reached_end() ? traits_type::eof() : traits_type::to_int_type(*gptr());
    }

    std::streampos seekoff(off_type offset, std::ios_base::seekdir way, std::ios_base::openmode which) override {
        if(which & std::ios_base::in) {
            if(way == std::ios_base::cur && buffer_size_read() > -offset && buffer_size_unread() > offset) {
                gbump(offset);
                return pos_type(base + buffer_size_read());
            } else if (way == std::ios_base::beg) {
                return seekpos(pos_type(offset), which);
            }
        }
        return invalid_position();
    }

    std::streampos seekpos(pos_type pos, std::ios_base::openmode which) override {
        if(which & std::ios_base::in && base < off_type(pos) && base + buffer_size_seekable() > off_type(pos)) {
            setg(eback(), eback() + (off_type(pos) - base), egptr());
            return pos_type(base + buffer_size_read());
        }
        return invalid_position();
    }

    bool has_reached_end() {
        return gptr() == egptr();
    }

    off_type buffer_size_read() {
        return off_type(gptr() - eback());
    }

    off_type buffer_size_unread() {
        return off_type(egptr() - gptr());
    }

    off_type buffer_size_seekable() {
        return off_type(egptr() - eback());
    }

    std::streampos invalid_position() {
        return pos_type(off_type(-1));
    }
};
