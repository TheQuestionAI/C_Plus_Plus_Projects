#include "vec3.h"
#include "ray.h"

#include <memory>

/*
    非完整类型的使用情景有限但能使用的场景却价值独特:
        1. 非完整类型的类可以定义指向这种类型的指针和引用(但不能够定义该类的对象).
        2. 可以声明(但是不能定义)以非完整类型作为参数或者返回类型的函数.
        3. 在非完整类型的类的定义域内, 这一非完整类型可以直接作为静态数据成员的声明类型.
*/
class material;     // 提前做材质类声明, 后面定义的hit_record结构体内部会使用指针指向.

/*
    record object, 它包含函数hit返回真时的交点参数t等数据.

    关于相交点法线向量的方向:  
        1. 我们可以始终让法线向量指向表面外point out. 即如果光线从外部与球体相交, 则法线和光线方向roughly相反. 如果射线从内部与球体相交, 则法线与光线方向roughly相同. 
           如果我们决定始终让法线指向表面外, 那么在着色过程时, 我们需要确定可视射线在表面的哪一侧. 
           我们可以通过将射线与法线进行比较来解决这一问题. 如果光线和法线的方向相同, 则该射线在对象内部; 如果光线和法线的方向相反, 则该射线在对象外部. 
           这可以通过取两个向量的内积来确定, 如果它们的内积为正，则射线在表面内侧; 如果它们的内积为负, 则射线在表面外侧.
        2. 或者, 我们可以使法线始终指向射线, 即法线方向与射线方向相反. 如果射线在球体外部, 则法线将指向外部; 但如果射线在球体内部, 则法线将指向内部.
           如果我们决定使法线始终指向射线, 则无法使用点积来确定射线位于曲面的哪一侧. 相反, 我们需要在hit_record结构体中存储可视射线是位于表面哪一侧这一信息.

    我们可以进行选择, 可以使法线始终指向曲面的“外部”, 或者始终指向入射光线. 该决定取决于我们是要在相交检测时还是在着色时确定射线位于表面的哪一侧.
    在本书中, 与几何类型相比, 我们拥有更多的材料类型, 因此我们将花费更少的工作, 并将确定可视射线在表面哪一侧放在相交检测时进行确定. 这只是一个优先事项, 您将在文献中看到这两种实现.
*/
// A Data Structure to Describe Ray-Object Intersections
struct hit_record {     // hit_record is just a way to stuff a bunch of arguments into a struct so we can send them as a group.
    point3 p;
    vec3 normal;            // 保存的是单位法向量.
    double t;
    bool front_face;        // 记录可视射线与表面的哪一侧相交. front_face为正, 则相交表面为外表面, 为负则为内部表面.

    // 用一个指针记录相交点的材质. 
    // 特别注意, material只有声明没有定义, 必然是incomplete type. 因此必须只能用指针而无法实例化对象.
    std::shared_ptr<material> mat_ptr;   

    void set_face_nomral(const ray& r, const vec3& outward_normal) {
        // 如果内积小于0, 那么和射线的相交表面的是内侧, front_face = false; 内积大于0, 那么和射线的相交表面是外侧, front_face = true.
        front_face = dot(r.direcion(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};