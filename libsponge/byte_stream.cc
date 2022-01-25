#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity):capacity_(capacity) { }

size_t ByteStream::write(const string &data) {
    size_t write_len = std::min(remaining_capacity(), data.size());
    stream = stream + data.substr(0, write_len);
    written_ += write_len;
    return write_len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t peek_len = std::min(len, stream.size());
    return stream.substr(0, peek_len);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) 
{
    size_t pop_len = std::min(len, stream.size());
    read_ += pop_len;
    stream.erase(stream.begin(), stream.begin()+pop_len);
    return ;
}


//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t read_len = std::min(len, stream.size());
    std::string read_data = stream.substr(0, read_len);
    read_+=read_len;
    stream.erase(stream.begin(), stream.begin()+read_len);
    return read_data;
}

void ByteStream::end_input() {stop_input = true;}

bool ByteStream::input_ended() const { return stop_input; }

size_t ByteStream::buffer_size() const { return stream.size(); }

bool ByteStream::buffer_empty() const { return stream.empty(); }

bool ByteStream::eof() const { return input_ended()&&buffer_empty(); }

uint64_t ByteStream::bytes_written() const { return written_; }

uint64_t ByteStream::bytes_read() const { return read_; }

size_t ByteStream::remaining_capacity() const { return capacity_ - stream.size(); }
