#ifndef UTILITY_H
#define UTILITY_H

// 在include头文件的时候, 一定不要出现头文件相互引用的死循环. 所以头文件的organization一定要严格顺序, 并且在源文件的include顺序也要正确合法.

// common header
#include "ray.h"
#include "vec3.h"
#include "hit_record.h"

#include <cmath>
#include <ctime>
#include <limits>
#include <memory>
#include <random>

// constants.
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// utility function.
// 角度转换为幅度.
inline double degrees_to_radian(double degrees) {
    return degrees * pi / 180;
}
// 幅度转换为角度.
inline double radian_to_degrees(double radian) {
    return radian * 180 / pi;
}

/*
    随机数发生器(随机数引擎+随机数分布函数)应定义为static类型从而使得每次发生器运行都能得到不同的随机数值.
    如果一个随机数发生器被定义成了普通类型, 即随机数引擎对象和关联的分布函数对象被定义成了普通类型, 那么在每次包含随机数发生器代码的函数被运行时, 它们都会返回相同的数值序列. 
    序列不变这一事实在调试时非常有用. 但是应用正式投入使用时则必须把随机数发生器定义为staitc静态类型, 这样可保证每次运行随机数发生器时, 都能得到不同的随机数值.
	
    设置随机数发生器种子: 把随机数发生器定义为staitc可有效的防止在应用程序运行阶段, 多次运行随机数发生器得到相同的随机数值; 
                         但是并没有解决应用程序被反复运行带来的随机数发生器得到相同随机数序列的问题.
                         通常希望每次运行应用程序都会生成不同的随机序列结果, 可以通过提供一个种子(seed)来达到这一目的. 
                         种子就是一个数值, 随机数引擎可以利用它从序列中一个新位置重新开始生产原始随机数.
    为引擎设置种子有两种方式:
		- 在创建随机数引擎对象时显式提供种子.
		- 调用随机数引擎对象的`seed()`成员函数设置种子.
	种子最优选择极其困难, 最常用方法调用系统函数time_t time( time_t* second );
    选择一个好的种子是极其困难的事情, 可能最常用的方法是调用系统函数time(). 这一函数定义在头文件<ctime>中, 它返回一个特定时刻到当前经历了多少秒. 
    函数time()接受单个指针参数, 它指向用于写入时间的数据结构. 如果此指针为空time(nullptr), 则函数简单的返回以秒为单位的当前时间.
		- 由于time()返回的是以秒为单位的时间, 所以如果应用程序的启动频繁性比秒级还高的话, 那么使用time()函数作为种子就无效.
*/
inline double random_double(const double vmin = 0.0, const double vmax = 1.0) {
    // 随机数发生器需要定义为静态类型, 并且设置良好的种子.
    // 随机数引擎是函数对象类, 定义了一个不接受参数的调用运算符(), 返回一个随机数引擎对象可以生成的unsigned类型原始整数, 取值范围内的每个整数被生成的概率相同, 服从均匀分布. 
    // 对于大多数场合, 随机数引擎函数对象的输出是不能直接使用的, 因为通常随机数引擎对象生成的随机数的值返回与需求范围不符合, 这也是为什么被称作原始随机数的原因.
    static std::mt19937 generator(time(nullptr));                               // 选择随机数引擎类, 并选择良好的种子.
    // 随机数分布函数对象类: 类似于随机数引擎类, 随机数分布类也是函数对象(模板)类. 
    // 分布函数对象类型定义了一个接受一个随机数引擎函数对象作为参数的调用运算符, 分布对象使用它的引擎参数生成符合自身概率分布的指定整数或浮点数类型(类模板T)随机数. 
    // 分布类型都是类模板, 具有单一的模板类型参数, 表示分布生成的随机数的数据类型, 这些分布要么生成随机整数要么生成随机浮点数, 
    // 每个分布类型都有一个默认模板实参, 生成浮点数的默认是double生成整数的默认是int.
    static std::uniform_real_distribution<double> distribution(vmin, vmax);       // 选择范围在[vmin,vmax]之间的双精度浮点数均匀分布函数.

    // 返回一个[vmin,vmax]范围内的随机双精度浮点数.
    return distribution(generator);
}

inline int random_int(const int vmin = 0, const int vmax = 1) {
    static std::mt19937 generator(time(nullptr));                               // 选择随机数引擎类, 并选择良好的种子.
    static std::uniform_int_distribution<int> distribution(vmin, vmax);       // 选择范围在[vmin,vmax]之间的整数均匀分布函数.

    // 返回一个[vmin,vmax]范围内的随机整数.
    return distribution(generator);
}

// 千万别uitility.h和vec3.h互相include, 把所有utility函数都定义在utility头文件中/
// 互相include 会导致不知道哪个先哪个后.

// 定义一个可以在给定中心为((vmim+vmax)/2, (vmim+vmax)/2, (vmim+vmax)/2), 边长为vmim+vmax的立方体内生成的随机点. 随机点函数都要定义为静态函数.
// 默认参数是生成一个随机双精度空间点, 该空间点位于中心在原点边长为2的立方体内.
inline vec3 random_vec3(const double vmin = -1.0, const double vmax = 1.0) {    
    return vec3(random_double(vmin, vmax), random_double(vmin, vmax), random_double(vmin, vmax));
}

// 定义一个随机空间点生成函数, 该随机点位于球心为原点半径为1的球的内部.
inline vec3 random_in_unit_sphere() {
    vec3 p;
    while(true) {
        p = random_vec3(-1.0, 1.0);     // 生成一个随机双精度空间点, 该空间点位于中心在原点边长为2的立方体内.
        if(p.lenth_squared() < 1)
            break; 
    }
    return p;
}

// 通过取一个中心在原点的单位球表面的随机点, 定义一个长度为1的随机单位向量.
inline vec3 random_unit_vector() {
    vec3 p = random_in_unit_sphere();
    // 对一个半径为1的球内部点做单位化, 就得到位于球表面的点, 同样也得到一个可以360度指向的长度为1的单位向量.
    return p / p.length();
}

// 定义一个以等概率(均匀分布)在一个和法向量同方向的半球内返回一个随机方向向量函数.
inline vec3 random_in_hemisphere(const vec3& normal) {
    vec3 in_unit_sphere = random_in_unit_sphere();      // 向量的大小跟它的方向没有关系, 所以我们可以直接使用球内均匀随机点函数, 而无需使用球表面均匀随机点函数.
    if(dot(normal, in_unit_sphere) > 0.0)   // 判断随机向量方向.
        return in_unit_sphere;
    return -in_unit_sphere;
}

// 在一个x-y平面或u-v平面的以原点为中心的单位圆上取随机空间点.
inline vec3 random_in_unit_disk() {
    vec3 p;
    while(true) {
        p = vec3(random_double(-1.0, 1.0), random_double(-1.0, 1.0), 0.0);  // // 在一个x-y平面或u-v平面的以原点为中心的单位圆上取随机点.      
        if(p.lenth_squared() < 1)
            break; 
    }
    return p;
}

// 定义一个clamp函数, 把值框定在一个范围内, 超过范围则就近取边界值.
inline double clamp(double x, double x_min, double x_max) {
    if(x < x_min) return x_min;
    if(x_max < x) return x_max;
    return x;
}

#endif