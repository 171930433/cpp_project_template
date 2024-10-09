#ifndef __LTTP_HPP__
#define __LTTP_HPP__

// https://github.com/parkertomatoes/lttb-cpp/blob/master/include/lttb.hpp

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <implot.h>
#include <message/message.h>

template <typename TPoint, typename TData, TData TPoint::*x, TData TPoint::*y>
struct LargestTriangleThreeBuckets {
  template <typename InputIt, typename OutputIt>
  static void Downsample(InputIt source, size_t sourceSize, OutputIt destination, size_t destinationSize) {
    if (destinationSize == 0 || sourceSize == 0) {
      return; // Nothing to do
    }

    if (destinationSize >= sourceSize) {
      std::copy_n(source, sourceSize, destination);
      return;
    }

    if (destinationSize == 1) {
      *destination = *source;
      return;
    }

    // Bucket size. Leave room for start and end data points
    // Note: We cast to double to avoid floored window propagation
    double every = static_cast<double>(sourceSize - 2) / static_cast<double>(destinationSize - 2);

    size_t aIndex = 0; // Initially a is the first point in the triangle

    // Always add the first point
    *destination = *source;
    ++destination;

    for (size_t i = 0; i < destinationSize - 2; ++i) {
      // Calculate point average for next bucket (containing c)
      TData avgX = 0;
      TData avgY = 0;
      size_t avgRangeStart = static_cast<size_t>((i + 1) * every) + 1;
      size_t avgRangeEnd = static_cast<size_t>((i + 2) * every) + 1;
      if (avgRangeEnd > sourceSize) { avgRangeEnd = sourceSize; }

      TData avgRangeLength = avgRangeEnd - avgRangeStart;

      for (; avgRangeStart < avgRangeEnd; ++avgRangeStart) {
        avgX += source[avgRangeStart].*x;
        avgY += source[avgRangeStart].*y;
      }
      avgX /= avgRangeLength;
      avgY /= avgRangeLength;

      // Get the range for this bucket
      size_t rangeOffs = static_cast<size_t>(TData(i + 0) * every) + 1;
      size_t rangeTo = static_cast<size_t>(TData(i + 1) * every) + 1;
      TData pointAX = source[aIndex].*x;
      TData pointAY = source[aIndex].*y;

      TData maxArea = -1;
      size_t nextAIndex = 0;

      for (; rangeOffs < rangeTo; ++rangeOffs) {
        // Calculate triangle area over three buckets
        TData area = std::abs(((pointAX - avgX) * (source[rangeOffs].*y - pointAY))
          - ((pointAX - source[rangeOffs].*x) * (avgY - pointAY)));
        // Note: Normally you need to also divide area by 2, but all we care about
        // is the largest area, so we can skip the division by a constant factor of 2
        if (area > maxArea) {
          maxArea = area;
          nextAIndex = rangeOffs; // Next a is this b
        }
      }

      *destination = source[nextAIndex]; // Pick this point from the bucket
      ++destination;
      aIndex = nextAIndex; // This a is the next a (chosen b)
    }

    *destination = source[sourceSize - 1]; // Always add last
    ++destination;
  }
};

template <typename _Sensor, typename _Scalar, int _XIndex = 13, int _YIndex = 14,
  std::enable_if_t<IsTrajectory<_Sensor>::value, void*> = nullptr>
struct LTTB {
  template <typename InputIt, typename OutputIt>
  static void Downsample(InputIt source, size_t sourceSize, OutputIt destination, size_t destinationSize) {
    if (destinationSize == 0 || sourceSize == 0) {
      return; // Nothing to do
    }

    if (destinationSize >= sourceSize) {
      std::copy_n(source, sourceSize, destination);
      return;
    }

    if (destinationSize == 1) {
      *destination = *source;
      return;
    }

    // Bucket size. Leave room for start and end data points
    // Note: We cast to double to avoid floored window propagation
    double every = static_cast<double>(sourceSize - 2) / static_cast<double>(destinationSize - 2);

    size_t aIndex = 0; // Initially a is the first point in the triangle

    // Always add the first point
    *destination = *source;
    ++destination;

    for (size_t i = 0; i < destinationSize - 2; ++i) {
      // Calculate point average for next bucket (containing c)
      _Scalar avgX = 0;
      _Scalar avgY = 0;
      size_t avgRangeStart = static_cast<size_t>((i + 1) * every) + 1;
      size_t avgRangeEnd = static_cast<size_t>((i + 2) * every) + 1;
      if (avgRangeEnd > sourceSize) { avgRangeEnd = sourceSize; }

      _Scalar avgRangeLength = avgRangeEnd - avgRangeStart;

      for (; avgRangeStart < avgRangeEnd; ++avgRangeStart) {
        auto frame = std::dynamic_pointer_cast<Message<_Sensor> const>(source[avgRangeStart]);
        avgX += frame->rpose_(0, 3);
        avgY += frame->rpose_(1, 3);
      }
      avgX /= avgRangeLength;
      avgY /= avgRangeLength;

      // Get the range for this bucket
      size_t rangeOffs = static_cast<size_t>(_Scalar(i + 0) * every) + 1;
      size_t rangeTo = static_cast<size_t>(_Scalar(i + 1) * every) + 1;
      auto frame = std::dynamic_pointer_cast<Message<_Sensor> const>(source[aIndex]);

      _Scalar pointAX = frame->rpose_(0, 3);
      _Scalar pointAY = frame->rpose_(1, 3);

      _Scalar maxArea = -1;
      size_t nextAIndex = 0;

      for (; rangeOffs < rangeTo; ++rangeOffs) {
        auto frame = std::dynamic_pointer_cast<Message<_Sensor> const>(source[rangeOffs]);

        // Calculate triangle area over three buckets
        _Scalar area = std::abs(
          ((pointAX - avgX) * (frame->rpose_(1, 3) - pointAY)) - ((pointAX - frame->rpose_(0, 3)) * (avgY - pointAY)));
        // Note: Normally you need to also divide area by 2, but all we care about
        // is the largest area, so we can skip the division by a constant factor of 2
        if (area > maxArea) {
          maxArea = area;
          nextAIndex = rangeOffs; // Next a is this b
        }
      }

      *destination = source[nextAIndex]; // Pick this point from the bucket
      ++destination;
      aIndex = nextAIndex; // This a is the next a (chosen b)
    }

    *destination = source[sourceSize - 1]; // Always add last
    ++destination;
  }
};

using PointLttb = LargestTriangleThreeBuckets<ImPlotPoint, double, &ImPlotPoint::x, &ImPlotPoint::y>;
using StateLttb = LTTB<State, double>;

#endif