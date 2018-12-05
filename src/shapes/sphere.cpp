/**
 * sphere.cpp
 *
 * By Sebastian Raaphorst, 2018.
 */

#include <memory>
#include <tuple>
#include <vector>

#include "constmath.h"
#include "intersection.h"
#include "ray.h"
#include "shape.h"
#include "sphere.h"
#include "vec.h"

using namespace raytracer::impl;

namespace raytracer::shapes {
    Sphere::Sphere(InstanceManager::dummy d) noexcept: Shape{d} {}

    std::shared_ptr<Sphere> Sphere::createSphere() noexcept {
        std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(dummy{});
        registerInstance(sphere);
        return sphere;
    }

    std::shared_ptr<Sphere> Sphere::createGlassSphere() noexcept {
        std::shared_ptr<Sphere> sphere = createSphere();
        sphere->getMaterial().setTransparency(1);
        sphere->getMaterial().setRefractiveIndex(1.5);
        return sphere;
    }

    const std::vector<Intersection> Sphere::localIntersection(const Ray &r) const noexcept {
        const auto sphere_to_ray = r.getOrigin() - predefined_tuples::zero_point;
        const auto &direction = r.getDirection();

        const auto a = direction.dot_product(direction);
        const auto b = 2 * direction.dot_product(sphere_to_ray);
        const auto c = sphere_to_ray.dot_product(sphere_to_ray) - 1;

        const auto discriminant = b * b - 4 * a * c;
        if (discriminant < 0)
        return {};

        const auto t1 = (-b - sqrtd(discriminant)) / (2 * a);
        const auto t2 = (-b + sqrtd(discriminant)) / (2 * a);
        if (t1 < t2) return {Intersection{t1, shared_from_this()}, Intersection{t2, shared_from_this()}};
        else return {Intersection{t2, shared_from_this()}, Intersection{t1, shared_from_this()}};
    }

    const Tuple Sphere::localNormalAt(const Tuple &point) const noexcept {
        return point - predefined_tuples::zero_point;
    }
}