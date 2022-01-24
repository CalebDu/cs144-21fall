#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // DUMMY_CODE(data, index, eof);
    _eof = _eof|eof;
    if(data.size()>_capacity)
        _eof = 0;
    
    if(data.empty()||index+data.size()-1<_firstUnassembleIndex)
    {
        if(_eof)
            _output.end_input();
        return ; 
    }

    // size_t _firstUnaccept = _firstUnassembleIndex + _output.remaining_capacity();
    std::string resData = data;
    size_t resIndex = index;
    if(resIndex<_firstUnassembleIndex)
    {
        resIndex = _firstUnassembleIndex;
        resData = data.substr(_firstUnassembleIndex - index);
    }
    // if(resIndex+resData.size()>_firstUnaccept)
    // {
    //     resData = resData.substr(0, _firstUnaccept - resIndex);
    // }

    Iter it = _unassemble.lower_bound(Unassemble(resData, resIndex));
    while(it!=_unassemble.begin())
    {
        if(it==_unassemble.end())
            it--;
        if(size_t delete_num = merge(resData, resIndex, it))
        {
            _unassemble_byte -= delete_num;
            if(it!=_unassemble.begin())
            {
                _unassemble.erase(it--);
            }else
            {
                _unassemble.erase(it);
                break;
            }
        }
        else
        {
            break;
        }
    }
    it = _unassemble.lower_bound(Unassemble(resData, resIndex));
    while(it!=_unassemble.end())
    {
        if(size_t delete_num = merge(resData, resIndex, it))
        {
            _unassemble_byte -= delete_num;
            _unassemble.erase(it++);
        }
        else
        {
            break;
        }
    }
    if(resIndex<=_firstUnassembleIndex)
    {
        size_t  written_len = _output.write(string(resData.begin() + _firstUnassembleIndex - resIndex, resData.end()));
        if(written_len==resData.size()&&eof)
            _output.end_input();
        else if(written_len!=resData.size())
        {
            std::string remain = resData.substr(_firstUnassembleIndex-resIndex+written_len);
            size_t remainIndex = _firstUnassembleIndex+written_len;
            _unassemble_byte+=remain.size();
            _unassemble.insert(Unassemble(remain, remainIndex));
        }
        _firstUnassembleIndex += written_len;
    }
    else
    {
        _unassemble.insert(Unassemble(resData, resIndex));
        _unassemble_byte += resData.size();
    }
    if(empty()&&_eof)
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const { return _unassemble_byte; }

bool StreamReassembler::empty() const { return _unassemble.empty(); }

size_t StreamReassembler::merge(std::string &data, size_t &index, Iter it) {
    size_t l1 = index, r1 = index + data.size() - 1;
    size_t l2 = it->index, r2 = it->index + it->data.size() - 1;
    std::string data2 = it->data;
    if (r1+1 < l2||r2+1<l1)
        return 0;

    index = std::min(l1, l2);
    size_t delete_num = data2.size();
    if(l1<=l2)
    {
        if(r1<r2)
        {
            data+=string(data2.begin()+r1-l2+1, data2.end());
        }
    }
    else
    {
        if(r1>r2)
        {
            data2+=string(data.begin()+r2-l1+1, data.end());
        }
        data = data2;
    }
    return delete_num;
}
