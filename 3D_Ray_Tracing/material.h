#ifndef MATERIAL_H
#define MATERIAL_H

// 在include头文件的时候, 一定不要出现头文件相互引用的死循环. 所以头文件的organization一定要严格顺序, 并且在源文件的include顺序也要正确合法.

// 我们应该把一些所有子类都会用到的头文件全都放在base class中include, 因为base class的头文件.必然会被子类所include.
#include "utility.h"    

// 定义一个材质抽象基类, 不同材质的反射, 折射系数是不同的.
class material {
    public:
        // 常量纯虚函数.
        virtual bool scatter(const ray& r, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
};

// 定义Lambertian材质子类.
class lambertian : public material {
    public:
        // explict default and parameter constructor
        explicit lambertian(const color& a = {}) : albedo{a} {}
    public:
        /*
            实现Lambertian reflection.
            Lambert物体: 当入射光线能量在所有方向均匀反射, 即入射能量以入射点为中心, 在整个半球空间内向四周各向同性的反射能量的现象, 称为漫反射, 一个完全的漫射体称为朗伯体.
            Lambertian漫反射模型假设反射光均匀分布在所有方向，简单方便.

            Lambert物体遵守Lambert漫反射余弦定理, 即物体表面的颜色c与表面法线和入射光线的夹角的余弦成比例.  
            法线与入射光方向重合的表面可获得最大光照强度, 法线与入射光方向相切(垂直)的表面不接受入射光照明,
            在两者之间的表面接收到的入射光照明强度与表面法线和入射光方向的夹角余弦值成正比 c ∝ (cos⁡(θ)=n∙l).
            漫反射导致的物体表面颜色值和漫反射光线的方向没有关系, 只与入射光线方向有关.

            对于Lambert材质, 我们定义一个散射函数来专门描述这种材质的入射光照射到材质表面的光的散射(包含漫反射, 镜面反射, 折射等等总称)情况.
            Lambert材质暂时只有漫反射term, 所以把散射函数定义为描述Lambertian漫反射, 当然在漫反射过程中入射光的强度有一部分被吸收, 剩下的才被漫反射出去.
            因此定义一个私有成员函数albedo来记录材质的反射率, 每单位强度的入射光, 只有albeda数量的光强度被漫反射出去.
        */
        virtual bool scatter(const ray& r, const hit_record& rec, color& attenuation, ray& scattered) const override {
            vec3 scatter_direction = Lambertian_diffuse_direction(rec.normal);     // 朗伯表面点, 漫反射/散射方向.
 
            if(scatter_direction.near_zero())       // 如果方向变为0, 那么重置为法线方向.
                scatter_direction = rec.normal;

            scattered = ray(rec.p, scatter_direction);      // 有了起点和方向, 就可以生成散射射线.
            attenuation = albedo;

            return true;
        }
    private:
        color albedo;       // 记录材质的反射率. albedo n. 反射率. 英文释义: The ratio of reflected to incident light. 就是对入射光的反射比率. 1单位强度的入射光的反射光线强度为albedo.

        // 定义Lamber漫反射函数, 返回漫反射的反射光线方向.
        vec3 Lambertian_diffuse_direction(const vec3& normal) const {
            // 我们对具有cos(theta)分布的Lambertian分布感兴趣. theta是入射光线与法线的夹角. (注: 我们是从可视射线方向逆推, 因此这些函数产生的漫反射方向实际上是入射光线方向.)
            // True Lambertian射线接近法线的可能性更高, 但是分布更均匀. 这是通过选择在单位球表面上沿表面法线偏移的随机点来实现的.
            // 可以通过在单位球体中选择随机点, 然后对其进行规范化来实现在单位球表面上选择随机点.
            return normal + random_unit_vector();  
            /*
                Lambertian漫反射相比于球内随机向量扰动法向量的random_diffuse方法光线的散射更加均匀, 朝法线散射方向的光线更少. 
                这意味着对于散射的对象, 它们会显得更亮, 因为更多的光向相机方向反弹(背景颜色), 而不是往大球射去再反弹(反弹越多光强越弱).
                对于阴影，较少的光直接向上反射，因此较大球体位于较小球体的正下方的部分会更亮.
            */  
        }
        /*
            实现随机漫反射.
            漫反射: 对于一个3D场景物体相交点, 漫反射就是把入射光线随机反射于四面八方的某一个方向. (注意: 可视射线才是漫反射方向, 而入射光线方向是确定方向, 但是我们是反向着来, 这也没什么问题.)
            
            那么使用何种随机方法来随机这一个方向. 首先需要明确反射方向必须是物体法线向量所指向空间的半球内方向, 不可能是与法线向量相反的指向物体表面的方向, 这违反常识.
            因此我们需要选取这样一个随机方向, 它的随机范围为法线向量所指向空间的半球方向.
            
            rec.normal存储的是相交点p的法线单位向量长度为1. 我们需要选取一个随机的漫反射方向, 所以我们可以给这个法线单位向量添加一个长度在1以内的随机扰动向量random_in_unit_sphere().
            这一累加向量就是一个可以确保和单位法向量同向的随机漫反射方向(也就是不会往单位法向量相反的指向表面的方向反射, 符合物理反射常识).
        */
        vec3 random_diffuse_direction(const vec3& normal) const {
            // point3 target = rec.p + rec.normal + random_in_unit_sphere();   =>   vec3 diffuse_dir = target - rec.p;
            // 因此漫反射射线的起点就是物体相交点p, 方向就是我们随机选择的满发射方向diffuse_dir.
            // 使用长度小于单位球半径的随机向量扰动单位法向量, 则扰动后的向量接近法线的概率高, 大幅度偏离法线的概率小. 生成的累加向量方向概率以(cos(theta))^3立方次幂权重分布
            return normal + random_in_unit_sphere();
        }

        vec3 uniform_difusse_direction(const vec3& normal) const {
            // 直接返回一个从均匀分布的半球空间内的随机向量即可.
            return random_in_hemisphere(normal);
        }
};

// 定义金属/镜面材质, 金属/镜面材质就是跟镜子一样反射.
class metal : public material {
    public:
        // explict default and parameter constructor
        explicit metal(const color& a = {}, const double f = 0.0) : albedo{a}, fuzz{f < 1 ? f : 1} {}
    public:
        /*
        实现specular reflection.
        具有一定光滑性的物体: 在真实世界中, 许多物体表面虽然本质不是绝对光滑但是却又有一定光滑性, 这就会使得物体表面显示出一定程度的光泽性, 产生高光highlights或镜面反射specular reflection. 
                             这些高光在物体表面上会随着视点的移动而位置改变. 如果我们仔细探究这些高光, 其实就能发现它们实际上就是镜面反射产生的反射光, 而反射光颜色就是光源发出的入射光线的颜色.
        镜面反射:  镜面反射是指若物体表面光滑, 当平行入射的光线射到这个表面时, 仍会平行地向一个方向反射出来, 并且入射光线和表面法线的夹角和反射光线和表面法线的夹角相等. 
                  这种反射就属于镜面反射. 镜面反射就只是将入射光线根据表面法线对称反射, 并且只在反射方向有能量其他方向均为0.
        */
        virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
            vec3 specular_reflect = specular_reflect_direction(unit_vector(r_in.direcion()), rec.normal);     // 镜面反射, 返回镜面反射的反射方向.

            // 有了起点和方向, 就可以生成散射射线.
            // 我们引入fuzzy relection模糊反射, 也就是说我们不希望所有镜面反射都是精确的, 因为现实世界的金属表面是不可能绝对光滑的, 不同的金属材质对于光线的镜面反射精确度是不同的.
            // 因此引入一个模糊系数[0,1]之间, 来乘以一个长度小于1的随机生成向量, 来扰动精确的镜面反射方向, 从而达到模糊反射效果.
            scattered = ray(rec.p, specular_reflect + fuzz*random_in_unit_sphere());      
            attenuation = albedo;

            return dot(scattered.direcion(), rec.normal) > 0;   // 判断镜面反射光是否和法线向量同向.
        }
    private:
        // 反照率（albedo）：是指表面在光源照射(辐射)的影响下，反射辐射通量与入射辐射通量的比值.
        color albedo;       // 记录材质的反照率. albedo n. 反照率. 英文释义: The ratio of reflected to incident light. 就是对入射光的反照比率. 1单位强度的入射光的反照光线强度为albedo.
        double fuzz;        // 记录金属表面的模糊反射系数. 对于金属球体而言, 球半径越大, 其模糊反射系数越大.

        /*
          定义镜面反射函数, 返回反射光方向.
          入射光v,   法向量n   镜面反射光s          
                        ^      ^
                  \     |     /
                   \    |    /
                    \   |   /
                     \  |  /
                      v | /
            ------------------------

            入射光v和n的夹角与反射光s与n的夹角完全相同.   v指向表面方向, s朝表面外指向.
            n是单位向量, 但代表入射光方向的v不一定是. 
            v = v|_ + v||, s = s|_ + s||
            求出v在法向量的投影向量, 即在法向量的分量(由投影公式可得)为v|| = dot(v,-n)*(-n)/||n||^2 = dot(v,n)*n
            又v|_ = v - v||
            镜面反射性质: s|_ = v|_ 且 s|| = -v||
            所以有s = v|_ + (-v||) = v - v|| - v|| = v - 2*dot(v,n)*n

            =>   s = v - 2*dot(v,n)*n
        */
        vec3 specular_reflect_direction(const vec3& incident, const vec3& normal) const {
            // v为入射光方向, v一定是单位向量.  如果不是在传入参数前单位化.
            // n为法线向量,  一定是单位向量.    如果不是则在传入参数前单位化.
            return incident - 2.0*dot(incident,normal)*normal;
        }
};

// 定义电介质材质. 电介质材质具有对光的折射性质.
/*
    透明材料（例如水，玻璃和钻石）是电介质。当光线撞击它们时，它会分解为反射射线和折射（透射）射线.
    我们将通过在反射或折射之间随机选择，并且每次交互仅生成一条散射射线来解决这一问题。
*/
class dielectric : public material {
    public:
        explicit dielectric(const double index_of_refraction = 1.0) : ir{index_of_refraction} {}
    public:
        /*
        实现dielectric refraction.
        折射refraction(维基百科): 在物理学中, 折射是指波在穿越介质或经历介质的渐次变化时传播方向上的改变. 光的折射是最容易观察的折射现象, 不过其他像是声音和海浪也都会有折射的性质.
        电介质dielectric: 电介质是一种可被电极化的绝缘体.
                          电介质可以折射光. 通常电介质具有足够的光滑性并且能被光线透射(折射)的透明物体. 钻石、玻璃、水和空气都是电介质. 
                          电介质具有滤光性. 滤光的意思是只允许某些特定波长范围内的光线透射, 而把波长不在这个范围内的光线反射回去, 允许透射的光波就被这一电介质过滤掉了, 
                                           从而电介质表面会显现出反射光线的颜色
        折射率refractive index: 介质的折射率n等于光在真空中的速率c跟光在介质中的速率v之比n=c/v

        全内反射total internal reflection.
        斯涅尔折射定律: sin(θ)*n1 = sin(φ)*n2, 所以有sin(φ) = sin(θ)*n1/n2;
        也即是说如果sin(θ)*n1/n2 > 1,, 那么φ就没有解, 也就是说此时没有折射光线, 发生全内反射.  
        i.e     if sin(θ)*n1/n2 > 1, 则全内反射(也就是镜面反射)
                else    发生折射.
        sin(θ) = sqrt(1 - cos^2(θ)). 因为θ为锐角, 必然取正值.

        或者使用折射光线在法线轴的分量来判断r|| = sqrt( 1 - (1-||v∙n||^2) * (n_1/n_2)^2 ) * n
        discrimnant = 1 - (1-||v∙n||^2) * (n_1/n_2)^2
        */
        virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
            
            // refraction_ratio是折射率之比n1/n2, 假设物体表面法向量是外部表面法线, 外部介质是空气(折射率=1.0), 光线从空气射入物体内部, 则 refraction_ratio = 1.0/n2, 即物体折射率的倒数.
            // 如果物体表面法向量是内部表面法向量, 那么光线从物体内部折射进入空气, 则refraction_ratio = n1/n2 = n1/1.0 = n1, 即物体的折射率. 
            double refraction_ratio = rec.front_face ? 1.0/ir : ir;

            // 首先判断是折射还是全内反射.
            vec3 r_in_unit_direction = unit_vector(r_in.direcion());
            double cos_theta = fmin(dot(-r_in_unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta*cos_theta);
            bool cannot_refract = refraction_ratio * sin_theta > 1.0;

            vec3 direction;
            // 判断是折射还是全内反射.
            // 因为我们这里只产生一条射线, 或者折射光线或者反射光线, 所以当求出反射比R之后, 随机选择一条是折射光线还是射线. 不做拆分成折射和反射两条光线处理.
            if(cannot_refract < 0  ||  reflectance(cos_theta, refraction_ratio) > random_double()) {
                // 全内反射 => 镜面反射, 返回镜面反射的反射方向.
                direction = specular_reflect_direction(r_in_unit_direction, rec.normal); 
            }
            else {
                // 折射
                direction = dielectric_refract_direction(r_in_unit_direction, rec.normal, refraction_ratio);
            }

            scattered = ray(rec.p, direction);      
            attenuation = color(1.0, 1.0, 1.0);     // 暂时假定如果电介质材质如果发生折射, 则所有光强度都折射, 没有反射或者吸收发生, 因此光强度减弱系数为1.

            return true;
        }
    private:
        // typically air = 1.0, glass = 1.3–1.7, diamond =2.4
        double ir;          // index of rafraction. 折射率.

        /*
            定义电介质折射函数, 返回折射光方向.
 
                       ^
                \      |
                 \     |
                  \    |                介质m1, 折射率n1, 入射光线v, 入射角θ, 入射光线和法线的夹角. 
                   \   |
                    \  |
                     v |
            -------------------------
                       | \
                       |     \          介质m2, 折射率n2, 折射光线r, 折射角φ, 折射光线与法线反方向的夹角. 有 n1 > n2 和 t2 > t1.
                       |         \
                       |             \
                       |                 \
                       |                     V    折射光线
            斯涅尔折射定律: sin(θ)*n1 = sin(φ)*n2;
            我们来求折射光线r的方向上的向量. 假设折射没有能量损失, 入射向量v和法线n都是单位向量.
            由三角函数恒等式sin^2⁡(φ)+cos^2⁡(φ)=1和φ为锐角立即有 => cos⁡(φ) = sqrt(1 - (n_1^2)*(1-cos^2⁡(θ))/n_2^2)
            由此可求得折射方向r = sin⁡(φ)b-cos⁡(φ)n 
                               = n_1*(v + cos⁡(θ)*n)/n_2 -cos⁡(φ)*n
                               = n_1 * (v-(v∙n)*n)/n_2   -   sqrt(1 - n_1^2*(1-||v∙n||^2)/n_2^2) * n

            r = r|_ + r||
            r|_ = (v-(v∙n)*n) * (n_1/n_2)    =>   (r|_)^2 = (n_1/n_2)^2 * (v-(v∙n)*n)∙(v-(v∙n)*n) = (n_1/n_2)^2 * (||v||^2 - 2||v∙n||^2 + ||v∙n||^2) = (n_1/n_2)^2 * (1 - ||v∙n||^2)
            r|| = sqrt( 1 - (1-||v∙n||^2) * (n_1/n_2)^2 ) * n = sqrt(1 - ||r|_||^2)

            全内反射total internal reflection.
            斯涅尔折射定律: sin(θ)*n1 = sin(φ)*n2, 所以有sin(φ) = sin(θ)*n1/n2;
            也即是说如果sin(θ)*n1/n2 > 1,, 那么φ就没有解, 也就是说此时没有折射光线, 发生全内反射.  
            i.e     if sin(θ)*n1/n2 > 1, 则全内反射(也就是镜面反射)
                    else    发生折射.
            sin(θ) = sqrt(1 - cos^2(θ)). 因为θ为锐角, 必然取正值.
        */
        vec3 dielectric_refract_direction(const vec3& incident, const vec3& normal, const double etai_over_etat) const {
            // etai_over_etat是折射率之比n1/n2, 假设物体表面外部介质是空气(折射率=1.0), 光线从空气射入物体内部, 则 etai_over_etat = 1.0/n2, 即物体折射率的倒数.
            // 如果光线从物体内部折射进入空气, 则etai_over_etat = n1/n2 = n1/1.0 = n1, 即物体的折射率. 
            double cos_theta = fmin(dot(-incident, normal), 1.0);     // cos_theta是入射光线向量与法线向量的锐角夹角.
            vec3 r_out_perpendicular = etai_over_etat * (incident + cos_theta*normal);
            // 这里可以直接简化使用r_out_perpendicular的值计算r_out_parallel.  使用fabs函数确保根号内的结果大于0.
            vec3 r_out_parallel      = -sqrt(fabs(1.0 - r_out_perpendicular.lenth_squared())) * normal;     
            return 
            r_out_perpendicular + r_out_parallel;
        }

        vec3 specular_reflect_direction(const vec3& incident, const vec3& normal) const {
            // v为入射光方向, v一定是单位向量.  如果不是在传入参数前单位化.
            // n为法线向量,  一定是单位向量.    如果不是则在传入参数前单位化.
            return incident - 2.0*dot(incident,normal)*normal;
        }

        /*
            基础反射率Base Reflectivity: 当垂直观察的时候, 任何物体或者材质表面都有一个基础反射率, 但是如果以一定的角度往平面上看的时候所有反光都会变得明显起来. 
                                        可以自己尝试一下, 用垂直的视角观察木制/金属桌面, 此时一定只有最基本的反射性. 
                                        但是如果你从近乎90度(是指可视方向和表面法线的夹角)的角度观察的话反光就会变得明显的多. 如果从理想的90度视角观察, 所有的平面理论上来说都能完全的反射光线. 
                                        这种现象因菲涅尔而闻名, 并体现在了菲涅尔方程之中.
            
            菲涅尔方程:  涅耳方程(Fresnel equation)描述了光线经过两个介质的界面时, 反射和透射(折射)的光强比重.
                        当光线撞击电介质时, 它会分解为反射射线和折射(透射)射线. 定义R(.)表示反射光强度和入射光强度的比值, 如果假设入射光强度为1, 那么反射光强度就为R(.).
                        根据菲涅尔公式, 电介质对光线反射量/折射量会随入射光角度不同而发生变化. (即: 反射出来的光线强度和折射出来的光线强度会随入射角度不同而各自不同但其总和是相同的). 

            Schlick近似: 要实现接近菲涅尔公式的效果, 求反射光与入射光强度比值R(即反射率)的一种好方法是使用Schlick近似
                                    R(h,v,F_0) = R_0 + (1.0 - R_0) * (1 - cos⁡(θ))^5, 
                         其中θ是入射角即入射光线和表面法线的夹角, R_0是介质表面的基础反射率(也即是以垂直视角/法线负方向观察表面时的反射率), 它可以使用相邻介质折射率计算出 
                                    R_0 = ( (n_1-n_2)/(n_1+n_2) )^2.

            Reflectance: 翻译为“反射率”, 是用来衡量物质反射能力的量, 定义为物体表面反射能量与到达物体表面入射能量的比率. 反射率是波长的函数, 且一般为正.
                        又称光谱反射率, 是波长的函数, 又称为光谱反射率ρ(λ), 定义为反射能与入射能之比.
            Albedo: 翻译为“反照率”, 是指目标物的出射能量与入射能量之比值, 或称半球反射率. 它是反演很多地表参数的重要变量, 反映了地表对太阳辐射的吸收能力.
            
            两者的区别： 1.reflectance是指某一波段向一定方向的反射, albedo是反射率在所有方向上的积分。
                        2.reflectance是波长的函数, 不同波长reflectance不一样; albedo是对全波长而言的.
        */
        static double reflectance(const double cosine, const double ref_index) {
            // 使用 Schlick's approximation for reflectance.
            // ref_index = n1/n2, 作为输入参数传递过来.
            double R0 = (1.0 - ref_index) / (1.0 + ref_index);
            R0 *= R0;

            return R0 + (1.0 - R0)*pow(1.0 - cosine, 5);         
        }
};

#endif