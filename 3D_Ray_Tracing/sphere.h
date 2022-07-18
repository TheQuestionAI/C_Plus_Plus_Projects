#ifndef SPHERE_H
#define SPHERE_H

#include "surface.h"
#include "vec3.h"

class sphere : public surface {
    public:
        // default and parameter constructor.
        sphere(const point3 cen = {}, const double r = 0.0, std::shared_ptr<material> m_ptr = nullptr) : center{cen}, radius{r}, mat_ptr{m_ptr} {}

    public:
        // 显示标注这是对抽象基类虚函数的覆盖, 前面使用virtual, 后面使用override.
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    private:
        point3 center;
        double radius;          // 这里其实可以允许半径为负数, 此时的是一个半径为|radius|的球, 但是它的表面正向法向量向内指示.
        std::shared_ptr<material> mat_ptr;      // 一个指针存放球表面的材质.
};

/*
    射线与sphere相交检测.
    一个球可以由一个中心point3 C和一个半径浮点数R表示. 任意满足(x-Cx)^2 + (y-Cy)^2 + (z-Cz)^2 = R^2的点都在球面上.
    化成向量形 dot(P-C, P-C) = R^2, 其中P = (x, y,z ), C=(Cx, Cy, Cz).
    射线P(t) = O + t*b, 代入球面方程化简后由: dot(b,b)*t^2 + 2*dot(b,O-C)*t + (dot(O-C,O-C)-R^2) = 0.
    对应标准一元二次方程 a*t^2 + bx + c = 0;
    a = dot(b,b);       b = 2*dot(b,O-C);       c = dot(O-C,O-C)-R^2.
    一元二次方程解的判别式delta = b^2 - 4ac.   如果delta>=0, 则有交点.
*/
bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    // 先求射线起点到球心的方向向量.
    vec3 oc = r.origin() - center;
    // 再求一元二次方法a, b, c; 然后求判别式. 我们可以优化代码.
    // 由相交点t = (-b - sqrt(b^2 - 4ac)) / (2a);
    // 假设b = 2h, 所以有-(2h + sqrt(4h^2 - 4ac))/(2a) = -(h + sqrt(h^2 - ac)) / a;
    // 所以 t = -(h + sqrt(h - ac)) / a. 给定 h = 0.5 * b = 0.5 * b * 2*dot(b,O-C) = dot(b,O-C)
    // 又a = dot(b,b) = ||b||^2; c = ||OC||^2 - R^2 i.e. 长度平方, 直接调用长度平方函数.
    double a = r.direcion().lenth_squared();
    double half_b = dot(r.direcion(), oc);
    double c = oc.lenth_squared() - radius*radius;
    
    double discriminant = half_b*half_b - a*c;
    // 通过判别式判断是否相交. 如果有相交点, 我们求取离视点最近的相交点的参数t = (-b - std::sqrt(discriminant)) / (2.0*a) < (-b + std::sqrt(discriminant)) / (2.0*a).
    // 我们需要相交点来计算相交点的法线向量.
    if(discriminant < 0) return false;

    // Find the nearest root that lies in the acceptable range.
    // 先判断离可视射线最近的root是否在range之中.
    // 效率考量优化代码, 只用< operator.
    double sqrtd = std::sqrt(discriminant);
    double root = (-half_b - sqrtd) / a;
    if(root < t_min  ||  t_max < root ) {
        root = (-half_b + sqrtd) / a;
        if(root < t_min  ||  t_max < root)
            return false;
    }

    // 如果相交点落在range之中, 那么填充rec对象保存相交点的各方面信息, 比如相交点3D空间位置, 射线方程t的值, 还有点的平面法线向量.
    // 判断相交表面是表面内侧还是外侧, 并始终记录方向始终指向射线的的法线.
    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = (rec.p - center) / radius;     // 求单位法向量.
    rec.set_face_nomral(r, outward_normal);
    rec.mat_ptr = mat_ptr;      // 也需要记录相交点的材质.
    
    return true;
}

#endif