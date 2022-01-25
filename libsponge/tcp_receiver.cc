#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();
    size_t len = seg.length_in_sequence_space();
    Buffer payload = seg.payload();
    uint64_t begin, end, firstUnassemble, firstUnaccept;  // absolute seqno
    
    if(!_syn_flag&&!header.syn)
        return;

    if (_syn_flag) {
        begin = unwrap(header.seqno, _isn, _checkpoint);
        end = begin + len - 1;
        firstUnassemble = unwrap(_ack_no, _isn, _checkpoint);
        firstUnaccept = firstUnassemble + window_size();
    } else {
        begin = 1;
        end = len;
        firstUnassemble = 1;
        firstUnaccept = window_size() + 1;
    }
    if (begin >= firstUnaccept || end < firstUnassemble)
        return;

    if (header.syn && !_syn_flag) {
        _syn_flag = true;
        _isn = header.seqno;
        _reassembler.recv_flag();
        // _ack_no = wrap(_reassembler.get_first_unassemble(), _isn);
    }
    if (header.fin) {
        _fin_flag = true;
    }
    std::string data = payload.copy();

    _reassembler.push_substring(data, begin, _fin_flag);
    if(_fin_flag&&_reassembler.empty())
        _reassembler.recv_flag();
    
    _checkpoint = _reassembler.get_first_unassemble() - 1;
    _ack_no = wrap(_reassembler.get_first_unassemble(), _isn);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_syn_flag)
        return _ack_no;
    else
        return nullopt;
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
