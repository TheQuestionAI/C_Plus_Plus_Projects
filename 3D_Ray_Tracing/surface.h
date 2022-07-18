#ifndef SURFACE_H
#define SURFACE_H

// 在include头文件的时候, 一定不要出现头文件相互引用的死循环. 所以头文件的organization一定要严格顺序, 并且在源文件的include顺序也要正确合法.

// 我们应该把一些所有子类都会用到的头文件全都放在base class中include, 因为base class的头文件.必然会被子类所include.
#include "utility.h"        // base class包含utility头文件, 所有子类在inlcude base class的时候自动包含. 

// 特别注意, extern修饰符是对变量或者说类对象做外部声明用, 例如extern material mat; 这才对.
// 对于class和struct本身无法使用extern修饰符, 只能直接class material; 声明一个material类但是不做定义, 此时material类是非完整类型incompete type.
// incomplete type只能被指针或者引用指向, 不能实例化对象.
class material;         // 让base class做材质类声明, 这样所有子类include基类就自动有了这一材质类声明.

/*
    光线跟踪器ray-tracer中的关键类层次结构是构成模型的几何表面. 
    在面向对象的实现中, 需要定义一个名为surface的抽象基类, 通过它可以派生出triangle、sphere、surface_list等子类, 并且它们全部都应该支持hit函数. 
    有了这样的抽象基类, 我们就创建一个可以具有一个通用接口的光线跟踪器, 该接口对建模几何图元几乎没有要求, 仅使用sphere球体就能对其进行调试.

    特别重要的一点是, 任何能够被射线“击中”的物体都应该被定义成surface的子类, 即使是一个表面集合列表surface_list也应被定义为surface的子类. 
    除此之外包括表示效率的数据结构, 例如层次包围体BVH, 由于它可以被射线相交, 所以它们也应被定义为surface的子类
*/
struct surface {
    public:
        // surface是抽象基类, 因此它内部的成员函数全部为纯虚函数. 抽象基类无法调用构造函数构建对象.
        // (t_min,t_max)是射线的区间, rec是一个通过引用传递的record object, 它包含函数hit返回真时的交点参数t等数据.
        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const  = 0;
};

#endif