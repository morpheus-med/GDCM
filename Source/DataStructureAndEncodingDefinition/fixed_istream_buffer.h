#include <streambuf>
#include <cstring>
#include <vector>
#include <istream>

/**
 The gdcmReader does seeking when decoding the headers so we need to buffer the input stream.
 You can seek back at least half the buffer size and at most the full buffer size depending on the sequence position.
 You may not be able to seek forward but can seek forward at most half the buffer size depending on the sequence position.
 Seeking will fail if the result is outside of the current buffer.
 This class performs a read on construction and so may block depending on the kind of istream used.
 This is based on https://stackoverflow.com/questions/44711735/someway-to-buffer-or-wrap-cin-so-i-can-use-tellg-seekg
*/
class fixed_istream_buffer : public std::streambuf {
    std::istream& stream;
    const unsigned size;
    const unsigned half;
    std::vector<char> buffer;
    std::streamoff base; //corresponds to the position of eback relative to the beginning of the input stream

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
        if(input_sequence(which)) {
            bool after_beginning = buffer_size_read() > -offset;
            bool before_end = buffer_size_unread() > offset;
            if(way == std::ios_base::cur && after_beginning && before_end) {
                gbump(offset);
                return pos_type(base + buffer_size_read());
            } else if (way == std::ios_base::beg && offset >= 0) {
                return seekpos(pos_type(offset), which);
            }
        }
        return invalid_position();
    }

    std::streampos seekpos(pos_type pos, std::ios_base::openmode which) override {
        bool after_beginning = base < off_type(pos);
        bool before_end = base + buffer_size_seekable() > off_type(pos);
        if(input_sequence(which) && after_beginning && before_end) {
            setg(eback(), eback() + (off_type(pos) - base), egptr());
            return pos_type(base + buffer_size_read());
        }
        return invalid_position();
    }

    bool input_sequence(std::ios_base::openmode which) {
        return which & std::ios_base::in;
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
