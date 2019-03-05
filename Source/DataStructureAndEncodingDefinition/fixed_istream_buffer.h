#include <streambuf>
#include <cstring>
#include <vector>
#include <istream>

/**
 The gdcmReader does seeking when decoding the headers so we need to buffer the input stream.
 You can seek back at least half the buffer size and at most the full buffer size depending on the sequence position.
 Seeking forward outside of the buffer will trigger reads until the buffer has been filled up to the seek position,
 at which point you can seek back the full buffer length.
 Seeking backwards will fail if the result is outside of the current buffer and seeking will ALWAYS fail when seeking std::ios::end.
 There is at least one case (implicit little endian without preamble or header) where gdcm will try to seek std::ios::end.
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
            bool after_beginning = buffer_size_read() >= -offset;
            bool before_end = buffer_size_unread() > offset;
            if(way == std::ios_base::cur) {
                if(after_beginning && before_end) {
                  gbump(offset);
                  return pos_type(base + buffer_size_read());
                } else if (after_beginning) {
                  return seek_offset_forward_via_read(offset);
                }
            } else if (way == std::ios_base::beg && offset >= 0) {
                return seekpos(pos_type(offset), which);
            }
        }
        return invalid_position();
    }

    std::streampos seekpos(pos_type pos, std::ios_base::openmode which) override {
        bool after_beginning = base <= off_type(pos);
        bool before_end = base + buffer_size_seekable() > off_type(pos);
        if(input_sequence(which)) {
            if(after_beginning && before_end) {
                setg(eback(), eback() + (off_type(pos) - base), egptr());
                return pos_type(base + buffer_size_read());
            } else if(after_beginning) {
                return seek_position_forward_via_read(pos);
            }
        }
        return invalid_position();
    }

    std::streampos seek_offset_forward_via_read(off_type offset) {
        std::streampos seekpos = base + buffer_size_read() + offset;
        return seek_position_forward_via_read(seekpos);
    }

    // this is potentially expensive, especially when reading immediatley after seeking
    // in the usual case this will not be called
    // assumes seekpos is past the end of the current buffer
    std::streampos seek_position_forward_via_read(std::streampos seekpos) {
        std::streampos endpos = base + buffer_size_seekable() - 1;
        unsigned total_read_length = seekpos - endpos;
        unsigned full_reads = total_read_length / size;
        for(unsigned i = 0; i < full_reads; ++i) {
          std::streamsize read = stream.read(buffer.data(), size).gcount();
          setg(buffer.data(), buffer.data() + read - 1, buffer.data() + read);
          base += size;
          if(!stream.good()) { return invalid_position(); }
        }

        unsigned partial_read = total_read_length % size;
        unsigned copy_length = size - partial_read;
        std::memmove(eback(), eback() + (size - copy_length), copy_length);
        std::streamsize read = stream.read(eback() + copy_length, partial_read).gcount();
        setg(eback(), eback() + copy_length + read - 1, eback() + copy_length + read);
        base += partial_read;
        return stream.good() ? pos_type(base + buffer_size_read()) : invalid_position();
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
