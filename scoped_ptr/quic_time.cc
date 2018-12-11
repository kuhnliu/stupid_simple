// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "quic_time.h"

#include <cinttypes>
#include <cstdlib>
#include <limits>

namespace quic {
QuicString QuicTime::Delta::ToDebugValue() const {
  const int64_t one_ms = 1000;
  const int64_t one_s = 1000 * one_ms;

  int64_t absolute_value = std::abs(time_offset_);

  // For debugging purposes, always display the value with the highest precision
  // available.
  char line[256]={0};
  if (absolute_value > one_s && absolute_value % one_s == 0) {
    sprintf(line,"%" PRId64 "s",time_offset_ / one_s);
    return QuicString(line);
  }
  if (absolute_value > one_ms && absolute_value % one_ms == 0) {
    sprintf(line,"%" PRId64 "ms", time_offset_ / one_ms);
    return QuicString(line);
  }
    sprintf(line,"%" PRId64 "us", time_offset_);
    return QuicString(line);
}

uint64_t QuicWallTime::ToUNIXSeconds() const {
  return microseconds_ / 1000000;
}

uint64_t QuicWallTime::ToUNIXMicroseconds() const {
  return microseconds_;
}

bool QuicWallTime::IsAfter(QuicWallTime other) const {
  return microseconds_ > other.microseconds_;
}

bool QuicWallTime::IsBefore(QuicWallTime other) const {
  return microseconds_ < other.microseconds_;
}

bool QuicWallTime::IsZero() const {
  return microseconds_ == 0;
}

QuicTime::Delta QuicWallTime::AbsoluteDifference(QuicWallTime other) const {
  uint64_t d;

  if (microseconds_ > other.microseconds_) {
    d = microseconds_ - other.microseconds_;
  } else {
    d = other.microseconds_ - microseconds_;
  }

  if (d > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
    d = std::numeric_limits<int64_t>::max();
  }
  return QuicTime::Delta::FromMicroseconds(d);
}

QuicWallTime QuicWallTime::Add(QuicTime::Delta delta) const {
  uint64_t microseconds = microseconds_ + delta.ToMicroseconds();
  if (microseconds < microseconds_) {
    microseconds = std::numeric_limits<uint64_t>::max();
  }
  return QuicWallTime(microseconds);
}

// TODO(ianswett) Test this.
QuicWallTime QuicWallTime::Subtract(QuicTime::Delta delta) const {
  uint64_t microseconds = microseconds_ - delta.ToMicroseconds();
  if (microseconds > microseconds_) {
    microseconds = 0;
  }
  return QuicWallTime(microseconds);
}

}  // namespace quic


