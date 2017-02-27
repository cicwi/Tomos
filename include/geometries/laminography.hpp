#pragma once

#include <limits>

#include "../geometry.hpp"
#include "../math.hpp"
#include "../volume.hpp"
#include "trajectory.hpp"

namespace tomo {
namespace geometry {

/**
 * A laminographic geometry
 *
 * \tparam D the dimension of the volume.
 * \tparam T the scalar type to use
 */
template <typename T>
class laminography : public trajectory<3_D, T> {
  public:
    /** Construct the geometry with a given number of lines. */
    laminography(volume<3_D, T> volume, int projection_count,
               math::vec<2_D, T> detector_size,
               math::vec<2_D, int> detector_shape,
               T relative_source_distance = (T)1.0,
               T relative_detector_distance = (T)1.0, T source_radius = (T)1.0,
               T detector_radius = (T)1.0)
        : trajectory<3_D, T>(volume, projection_count, detector_size,
                             detector_shape),
          relative_source_distance_(relative_source_distance),
          relative_detector_distance_(relative_detector_distance),
          source_radius_(source_radius), detector_radius_(detector_radius) {}

    math::vec<3_D, T> source_location(int projection) const override final {
        auto pivot = image_center_() -
                     relative_source_distance_ * this->volume_[1] *
                         math::standard_basis<3_D, T>(1);

        return pivot +
               apply_rotation_(source_radius_ * math::standard_basis<3_D, T>(0),
                               projection);
    }

    math::vec<3_D, T> detector_location(int projection) const override final {
        auto pivot = image_center_() +
                     relative_detector_distance_ * this->volume_[1] *
                         math::standard_basis<3_D, T>(1);

        return pivot + apply_rotation_(-detector_radius_ *
                                           math::standard_basis<3_D, T>(0),
                                       projection);
    }

    std::array<math::vec<3_D, T>, 2>
    detector_tilt(int projection) const override final {
        auto a = math::standard_basis<3_D, T>(1);
        auto b = math::normalize(detector_location(projection) -
                                 source_location(projection));
        auto n = math::cross<T>(a, b);
        auto n_norm = math::norm<3_D, T>(n);
        auto theta = math::asin<T>(n_norm);
        auto n_hat = n / n_norm;

        return {math::rotate(math::standard_basis<3_D, T>(0), n_hat, theta),
                math::rotate(math::standard_basis<3_D, T>(2), n_hat, theta)};
    }

    T& relative_source_distance() { return relative_source_distance_; }
    T& relative_detector_distance() { return relative_detector_distance_; }
    T& source_radius() { return source_radius_; }
    T& detector_radius() { return detector_radius_; }

  private:
    T relative_source_distance_;
    T relative_detector_distance_;
    T source_radius_;
    T detector_radius_;

    inline math::vec<3_D, T> transform_location_(math::vec<3_D, T> location,
                                                 int projection) const {
        return apply_rotation_(location - image_center_(), projection) +
               image_center_();
    }

    inline math::vec<3_D, T> apply_rotation_(math::vec<3_D, T> location,
                                             int projection) const {
        static auto axis = math::standard_basis<3_D, T>(1);

        T angle_projection = (T)2.0 * math::pi<T> / this->projection_count_;

        return math::rotate(location, axis, angle_projection * projection);
    }

    inline math::vec<3_D, T> image_center_() const {
        math::vec<3_D, T> image_center =
            this->volume_.origin() + (T)0.5 * this->volume_.physical_lengths();

        return image_center;
    }
};

} // namespace geometry
} // namespace tomo
