#ifndef HITTABLE_H
#define HITTABLE_H

#include "aabb.h"

class material;

class hit_record {
    public:
        point3 p;
        vec3 normal;
        shared_ptr<material> mat;
        double t;
        double u;
        double v;
        bool front_face;

        /* Sets the hit record normal vector*/
        /*Note: the paramater outward_normal is issumae to have unit length*/
        void set_face_normal(const ray& r, const vec3& outward_normal) {
            front_face = dot(r.direction(), outward_normal) < 0;
            normal = front_face ? outward_normal : -outward_normal;
        }
};

class hittable {
    public:
        virtual ~hittable() = default;

        virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;

        virtual aabb bounding_box() const = 0;
};

class translate : public hittable {
    public:
        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // Move the ray backwarsd by the offset
            ray offset_r(r.origin() - offset, r.direction(), r.time());

            // Determine whether an intersection exists along the offset ray (and if so, where)
            if (!object->hit(offset_r, ray_t, rec))
                return false;
            
            // Move the intersetion points forwarsd by the offset
            rec.p += offset;
            
            return true;
        }

    private:
        shared_ptr<hittable> object;
        vec3 offset;
};

#endif