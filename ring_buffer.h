pragma once

include <Arduino.h>

template<typename T, size_t N>
class RingBuffer {
public:
    RingBuffer() : head(0), count(0) {}

    void push(const T& v) {
        buf[head] = v;
        head = (head + 1) % N;
        if (count < N) count++;
    }

    bool empty() const { return count == 0; }
    size_t size() const { return count; }

    const T& back(size_t i = 0) const {
        size_t idx = (head + N - 1 - i) % N;
        return buf[idx];
    }

    bool popOldest(T& out) {
        if (count == 0) return false;
        size_t oldest = (head + N - count) % N;
        out = buf[oldest];
        count--;
        return true;
    }

private:
    T buf[N];
    size_t head;
    size_t count;
};
