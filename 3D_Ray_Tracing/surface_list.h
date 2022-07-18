#ifndef SURFACE_LIST_H
#define SURFACE_LIST_H

#include "surface.h"

#include <memory>
#include <vector>

class surface_list : public surface {
    public:
        // default and parameter constructor.
        explicit surface_list(const std::shared_ptr<surface> object = nullptr) { if(object) add(object); }

        // 还可以定义其他parameter构造函数, 但是没有定义, 先这样. Big-Five使用合成版本.
    public:
        void clear() { objects.clear(); }
        
        // 注意add函数的参数不加const, 会push进入vector中, 必须和vector元素类型相同.
        // 智能指针开销小, 所以直接pass by copy, 内部也直接使用push_back即可.
        void add(std::shared_ptr<surface> object) { objects.push_back(object); }

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    private:
        std::vector<std::shared_ptr<surface>> objects;
};

bool surface_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    // 给定一系列hittable object, 然后记录离视点最近的相交物体点.
    hit_record temp_rec;
    bool hit_anything = false;
    double closed_so_far = t_max;
    // 遍历迭代判断.
    for(const auto& object : objects) {
        if(object->hit(r, t_min, closed_so_far, temp_rec)) {
            hit_anything = true;
            closed_so_far = temp_rec.t;         // 一直在缩小closed_so_far所表示的区间最大值.
            rec = temp_rec;                     // 持续更新rec.
        }
    }

    return hit_anything;
}

#endif