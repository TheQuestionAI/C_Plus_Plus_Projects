#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

class vec3 {
    public:
        // parameter and default constructor
        vec3(const double e1 = 0.0, const double e2 = 0.0, const double e3 = 0.0) : e{e1, e2, e3} {}

        // 我们只显示定义了parameter constructor, 因此Big-five都使用默认合成版本.
    public:
        // 这一pulic区定义运算符重载函数.

        vec3 operator-() const { return {-e[0], -e[1], -e[2]}; }                    // 重载取负运算符.
        // 对于return type是vec3本身, 并且作用于同一object必须return by reference, 并且函数内部return语句是return *this
        vec3& operator+=(const vec3& v) {                                    
            e[0] += v[0];
            e[1] += v[1];
            e[2] += v[2];
            return *this;
        }
        vec3& operator-=(const vec3& v) {                                    
            // return *this += (-v);       // 貌似这样做并不比使用内置类型复合运算符-=快.
            e[0] -= v[0];
            e[1] -= v[1];
            e[2] -= v[2];
            return *this;
        }
        vec3& operator*=(const double t) {                                    
            e[0] *= t;
            e[1] *= t;
            e[2] *= t;
            return *this;
        }
        vec3& operator/=(const double t) { return *this *= 1/t; }       // 解引用运算符*的优先级要大于所有算术运算符, 算术运算符优先级顺序要大于所有复合运算符+=,-=,/=等等.

        // 当调用operator[]函数的vec3 object是常量对象时, 那么编译器自动启用accessor版本, 如果非常量vec3对象那么最佳匹配mutator版本.
        double& operator[](const int i) { return e[i]; }                      // mutator版本必须要使用return by reference, 这样返回的element的state才能被更改.
        const double& operator[](const int i) const { return e[i]; }          // accesorr函数内部不能改变vec3 object的state, 返回const vec3 state.
        //double operator[](const int i) const {return e[i];}                 // accessor版本有两种实现方式, 对于返回值是内置类型而言, 可以直接return by copy.

    public:
        double x() const { return e[0]; }     
        double y() const { return e[1]; }  
        double z() const { return e[2]; }

        // 求长度的平方和求长度具有同样的效果, 任何只是使用长度做判断的地方都可以使用长度平方, 减少求根号所带来的大量计算消耗.
        double lenth_squared() const { return e[0]*e[0] + e[1]*e[1] + e[2]*e[2]; }
        double length() const { return std::sqrt(lenth_squared()); }

        // 定义一个函数判断向量是否是零向量, 我们把所有分量值小于1e-8的向量看做是趋近于0的向量, 或者就是数值定义下的零向量.
        bool near_zero() const {
            const double eps = 1e-8;
            return (fabs(e[0]) < eps) && (fabs(e[1]) < eps) && (fabs(e[2]) < eps);
        }

    private:
        // 使用一个包含3个元素的array表示vector. We use double here, but some ray tracers use float.
        // 从c++ primer一书, 建议用double, 因为现在编译器对double双精度浮点数的优化其运算效率已经不弱于float, 甚至很多时候还有效率更高.
        double e[3];    
};

// 定义两个类型别名, 用vec3表示color和point. 通常这两个类应该稍有区别于vec3, 但是这里我们为了简单化直接定义类型别名.
using color = vec3;     // RGB color
using point3 = vec3;    // 3D point

/*
    和vec3类一起的还应该定义uitiliy函数, 这也属于vec3类提供的接口, 不仅仅是vec3类的成员函数是接口. 
    主要提供的是对vec3的一些通用算术运算符和输入输出函数.
    
    源自C++ primer, 重载运算符作为成员还是非成员函数:
        1. 赋值运算符=, 下标运算符[], 调用运算符()和箭头成员访问运算符->必须定义为类成员. 如果这些操作符重载函数是全局友元的, 也就是说没有了该函数的左操作数是this指针的限制, 
           那么程序员可以任意定义左操作数的类型, 比如就会出现6=c, 6(c), 6[c], 6->c的代码，显然这样的代码是完全不符合正常语法规则的, 这是一个基于书写规范的原因. 
           对于赋值运算符如果不定义成成员函数, 那么编译器也会隐式合成一个赋值成员函数, 编译器会优先使用成员函数版本, 这样就容易发生意想不到的错误, 比如浅拷贝.
        2. 复合赋值运算符(+=,-=,*=,/=等等)应该定义为类成员与赋值运算符=成员特性相匹配, 但不是强制要求.
        3. 改变对象状态的运算符或者与给定类型密切相关的运算符, 如递增运算符++, 递减运算符--和解引用运算符*通常应该定义为类成员.
        4. 具有对称性的运算符可能转换任意一端的运算对象, 如算术运算符(+,-,*,/等)、相等性运算符(==,!=)、关系运算符(&&,||等)和位运算符(&,|等), 通常应该定义为inline非成员函数.
        5. 如果想提供含有类对象的混合类型表达式, 则重载运算符必须定义为非成员函数.
        6. 输入输出运算符<<和>>应该定义成inline非成员函数.
*/

// 如果utility函数想要使用vec3类的data member的话, 需要声明为友元函数.

// 全部定义为inline函数! 特别注意, 所有向量的utility函数都是返回结果的副本！！！return by copy.
inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    // vec3只有三个element, 所以直接索引输出, 无需迭代.
    // 重载输入输出运算符绝不会加std::endl/flush/ends, 保持和标准输出std::cout一致.
    // 运算顺序从右往左.
    return out << v[0] << ' ' << v[1] << ' ' << v[2];
}

// 定义向量的算术与运算符.
inline vec3 operator+(const vec3& u, const vec3& v) { return vec3(u[0]+v[0], u[1]+v[1], u[2]+v[2]); }
inline vec3 operator-(const vec3& u, const vec3& v) { return vec3(u[0]-v[0], u[1]-v[1], u[2]-v[2]); }
inline vec3 operator*(const vec3& u, const vec3& v) { return vec3(u[0]*v[0], u[1]*v[1], u[2]*v[2]); }
inline vec3 operator*(const double t, const vec3& v) { return vec3(t*v[0], t*v[1], t*v[2]); }
inline vec3 operator*(const vec3& v, const double t) { return vec3(t*v[0], t*v[1], t*v[2]); }
inline vec3 operator/(const vec3& v, const double t) { return vec3((1/t)*v[0], (1/t)*v[1], (1/t)*v[2]); }

// 定义向量的内积和外积.
inline double dot(const vec3& u, const vec3& v) { return u[0]*v[0] + u[1]*v[1] + u[2]*v[2]; }
inline vec3 cross(const vec3& u, const vec3& v) { return vec3(u[1]*v[2] - u[2]*v[1], u[2]*v[0] - u[0]*v[2], u[0]*v[1] - u[1]*v[0]); }

// 定义向量的标准化, 单位化, 返回的是副本.
inline vec3 unit_vector(const vec3& v) { return v / v.length(); }

#endif