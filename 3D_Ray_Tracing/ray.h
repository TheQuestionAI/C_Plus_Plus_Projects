#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
    public:
        // default and parameter constructor.
        ray(const point3& origin = {}, const point3& direction = {}) : orig{origin}, dir{direction} {}

         // 我们只显示定义了parameter constructor, 因此Big-five都使用默认合成版本.
    public:
        point3 origin() const { return orig; }
        vec3 direcion() const { return dir; }

        // input一个参数t, 得到ray上的一个点.
        point3 at(const double t) const { return orig + t*dir; }        // 注意这个函数是accesor, 要添加const指定是常量成员函数.

    private:
        // 一条射线由两部分组成, 一个起始点, 一个方向向量
        point3 orig;
        vec3 dir;
};

#endif