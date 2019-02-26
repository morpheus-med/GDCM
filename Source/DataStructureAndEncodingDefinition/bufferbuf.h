#include <streambuf>
#include <cstring>

/**
 gdcmReader does seeking when decoding the headers so we need to buffer the input stream
 https://stackoverflow.com/questions/44711735/someway-to-buffer-or-wrap-cin-so-i-can-use-tellg-seekg
*/
class bufferbuf
    : public std::streambuf {
    enum { size = 2000, half = size / 2 };
    char            buffer[size];
    std::streambuf* sbuf;
    std::streamoff  base;
public:
    bufferbuf(std::streambuf* sbuf): sbuf(sbuf), base() {
        auto read = sbuf->sgetn(this->buffer, size);
        this->setg(this->buffer, this->buffer, this->buffer + read);
    }
    int underflow() {
        if (this->gptr() == this->buffer + size) {
            std::memmove(this->eback(), this->eback() + half, half);
            base += half;
            auto read = sbuf->sgetn(this->eback() + half, half);
            this->setg(this->eback(), this->eback() + half, this->eback() + half + read);
        }
        return this->gptr() != this->egptr()
            ? traits_type::to_int_type(*this->gptr())
            : traits_type::eof();
    }
    std::streampos seekoff(off_type                offset,
                           std::ios_base::seekdir  whence,
                           std::ios_base::openmode which) override {
        if (this->gptr() - this->eback() < -offset
            || this->egptr() - this->gptr() < offset
            || whence != std::ios_base::cur
            || !(which & std::ios_base::in)) {
            return pos_type(off_type(-1));
        }
        this->gbump(offset);
        return pos_type(this->base + (this->gptr() - this->eback()));
    }
    std::streampos seekpos(pos_type pos, std::ios_base::openmode which) override {
        if (off_type(pos) < this->base
            || this->base + (this->egptr() - this->eback()) < off_type(pos)
            || !(which & std::ios_base::in)) {
           return pos_type(off_type(-1));
        }
        this->setg(this->eback(), this->eback() + (off_type(pos) - this->base), this->egptr());
        return pos_type(base + (this->gptr() - this->eback()));
    }
};
