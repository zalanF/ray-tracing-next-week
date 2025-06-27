#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable {
    public: 
        // Stationary sphere
        sphere(const point3& static_center, double radius, shared_ptr<material> mat)
        : center(static_center, vec3(0, 0, 0)), radius(std::fmax(0,radius)), mat(mat) {}

        // Moving sphere
        sphere(const point3& center1, const point3& center2, double radius, shared_ptr<material> mat)
        : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat) {}


        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            point3 current_center = center.at(r.time());
            vec3 oc = current_center - r.origin();
            auto a = r.direction().length_squared();
            auto h = dot(r.direction(), oc);
            auto c = oc.length_squared() - radius*radius;

            auto discriminant = h*h - a*c;
            if (discriminant < 0)
                return false;

            auto sqrtd = std::sqrt(discriminant);

            /*Find the nearest root that lies in the acceptable range
              Check if ray is outside the range first; evaluate as false if true */
            auto root = (h - sqrtd) / a;
            if (!ray_t.surrounds(root)) {
                root = (h + sqrtd) / a;
                if (!ray_t.surrounds(root))
                    return false;
            }

            rec.t = root;
            rec.p = r.at(rec.t);
            //for a sphere, divifing by radius will normalize
            vec3 outward_normal = (rec.p - current_center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.mat = mat;

            return true;
        }

    private:
        ray center;
        double radius;
        shared_ptr<material> mat;
};

#endif