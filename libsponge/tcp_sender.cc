#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <iostream>
#include <random>
// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _timer(_initial_retransmission_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _byte_in_flight; }

void TCPSender::fill_window() {
    TCPSegment seg;
    if (_next_seqno == 0) {
        seg.header().syn = 1;
        _syn = 1;
        send_segment(seg);
        return;
    }
    if (_next_seqno == _byte_in_flight)
        return;
    size_t win_size = _win==0? 1: _win; 
    size_t remain;
    while ((remain = static_cast<size_t>(_recv_ackno - _next_seqno) + win_size)) {
        if (_stream.eof() && !_fin) {
            seg.header().fin = true;
            _fin = true;
            send_segment(seg);
            return;
        } else if (_stream.eof()) {
            return;
        } else {
            std::string data = _stream.read(std::min(TCPConfig::MAX_PAYLOAD_SIZE, remain));
            size_t read_len = data.size();
            seg.payload() = Buffer(std::move(data));
            if (_stream.eof()&&read_len!=remain) {
                seg.header().fin = true;
                _fin = true;
            }
            if (seg.length_in_sequence_space() == 0)
                return;
            send_segment(seg);
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    // DUMMY_CODE(ackno, window_size);

    _win = window_size;
    uint64_t recv_ackno = unwrap(ackno, _isn, _next_seqno);
    if (recv_ackno <= _recv_ackno)
        return;
    if (recv_ackno > _next_seqno)
        return;

    _recv_ackno = recv_ackno;
    _consecutive_retransmission = 0;
    _timer._rto = _timer._irto;
    _timer._last_tick = 0;
    // std::cout<<_segments_out.size()<<endl;
    while (!_segments_outstanding.empty()) {
        TCPSegment front = _segments_outstanding.front();
        uint64_t seq = unwrap(front.header().seqno, _isn, _next_seqno);
        if (_recv_ackno - seq >= front.length_in_sequence_space()) {
            _byte_in_flight -= front.length_in_sequence_space();
            _segments_outstanding.pop();
        } else {
            break;
        }
    }

    fill_window();
    _timer.start();

    return;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    //  DUMMY_CODE(ms_since_last_tick);
    if (_timer.tick(ms_since_last_tick)) {
        if (!_segments_outstanding.empty()) {
            TCPSegment &seg = _segments_outstanding.front();
            _segments_out.push(seg);
            if (_win) {
                _consecutive_retransmission++;
                _timer._rto *= 2;
            }
            _timer.start();
        } else {
            _timer.close();
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmission; }

void TCPSender::send_empty_segment() {
    TCPSegment empty_seg;
    empty_seg.header().seqno = next_seqno();
    _segments_out.push(empty_seg);
}

void TCPSender::send_segment(TCPSegment &seg) {
    seg.header().seqno = next_seqno();
    _next_seqno += seg.length_in_sequence_space();
    _byte_in_flight += seg.length_in_sequence_space();

    _segments_out.push(seg);
    _segments_outstanding.push(seg);
    _timer.start();
}

void Timer::start() {
    if (run)
        return;
    run = true;
    _last_tick = 0;
    // _rto = _irto;
}
void Timer::close() {
    if (!run)
        return;
    _last_tick = 0;
    run = true;
    // _rto = _irto;
}
bool Timer::tick(const size_t ms_since_last_tick) {
    if (!run)
        return 0;
    if (_last_tick + ms_since_last_tick < _rto) {
        _last_tick += ms_since_last_tick;
    } else {
        _last_tick = _rto;
    }
    if (_last_tick >= _rto) {
        _last_tick = 0;
        return 1;
    }
    return 0;
}