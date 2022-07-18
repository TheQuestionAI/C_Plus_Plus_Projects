// 在include头文件的时候, 一定不要出现头文件相互引用的死循环. 所以头文件的organization一定要严格顺序, 并且在源文件的include顺序也要正确合法.
// 尤其是对main.cc源文件, 最终这一main程序所需的所有头文件(包含的函数, 定义, 类)都会全部被编译器编译到这一main文件中, 然后生成可执行.exe文件.
#include "utility.h"

#include "camera.h"
#include "color.h"
#include "material.h"  
#include "surface_list.h"
#include "sphere.h"
     
#include <iostream>
/*
    ray tracer光线追踪器的核心是使从视点发出的光线穿过2D成像平面像素并计算沿这些光线方向看到的空间场景点的颜色. 涉及的步骤是
        1. 计算从视点发出的闯过2D成像平面像素的光线.
        2. 确定光线与3D空间中相交的物体点.
        3. 计算该交点p(t)的颜色.
*/
// 定义ray_color()函数, 该函数返回穿过像素点的可视射线所看到的物体颜色.
color ray_color(const ray& r, const surface& world, int depth) {
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if(depth <= 0) return color(0.0, 0.0, 0.0);

    // 如果world是球, 则先判断是否与球相交, 如果相交则返回相交点参数t, 用于计算相交点法线向量. 然后根据法线向量三个分量的值来计算color map. 如果不相交则返回background color.
    // 假定球的中心就在成像平面原点处C = (0, 0, -1), 半径R = 0.5. 并且球表面的颜色为纯红color = [1.0, 0.0, 0.0]/
    hit_record rec;
    // Some of the reflected rays hit the object they are reflecting off of not at exactly t = 0, 
    // but instead at t = -0.0000001 or t = 0.0000001 or whatever floating point approximation the sphere intersector gives us. 
    // So we need to ignore hits very near zero, set starting point of intersection range at t = 0.001.
    if(world.hit(r, 0.001, infinity, rec)) {  // infinity表示正无穷, 定义于utility.h头文件中.
        // 如果相交的话, 那么就有反射, 漫反射或者镜面反射, 依材质而定.
        // scatter()函数根据材质不同反射形式也不一样, 如果是Lambert材质那就是漫反射, 如果是metal材质那就是镜面反射.
        // scatter散射这里指的是漫反射, 镜面反射, 折射和全内反射的总称.
        ray scattered;      // 记录相交点的散射射线, 作为递归光线追踪所用.
        color attenuation;  // 光强减弱系数, 这里直接等于albedo, 也就是attenuation = albeda, 反射率直接刻画光强减弱系数.
        if(rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            // 乘以attenuation, 表示物体吸收了( 1.0 - attenuation )的光照强度,另外attenuation数量光强被scatter了出去. 不同材质的光反射率albedo不同.
            // 这里进行光线递归scatter直到超过depth限定范围或者没有物体再相交. 没有物体相交意味着射线最终反射射向远方. 此时获得的是image background color.
            // 可视射线和物体相交的次数越多那么最终反射的光强度越弱, 相交超过depth次数直接置反射光强度为0, 也就是这一像素点为纯黑色.
            // 这里对于光源和光源方向是怎么假设的? 貌似并没有说明光源方向和光源强度.
            return attenuation * ray_color(scattered, world, depth - 1);
        }
        return color(0.0, 0.0, 0.0);    // 如果无scatter射线, 则color为0. 一旦color为0那么像素点颜色必然为纯黑色. 0乘以任何递归过程的数认为0.
    }

    // 如果不相交则返回background color.
    vec3 unit_direction = unit_vector(r.direcion());    // 得到r方向上的单位向量
    // 2D成像平面是x-y平面. 这种取参数t值的方法, 不同长度的射线单位化之后的单位向量的x,y,w值是不同的.
    double t = 0.5*(unit_direction.y() + 1.0);        
    // [0.5, 0.7, 1.0] 天蓝色, [1.0, 1.0, 1.0] 纯白色. 让射线返回的颜色在纯白色和天蓝色范围内线性差值选择.
    // When t = 1.0 we want blue; When t = 0.0 we want white. In between, we want a white and blue blend color.
    // 线性差值公式永远是, lerp(t) = (1.0 - t)*startValue + t*endValue.
    return (1.0 - t)*color(1.0, 1.0, 1.0) + t*color(0.5, 0.7, 1.0);     
}

surface_list random_scene() {
    surface_list world;
    
    auto ground_material = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());
            
            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<material> sphere_material;
                
                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = random_vec3() * random_vec3();
                    sphere_material = std::make_shared<lambertian>(albedo);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                } 
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random_vec3(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = std::make_shared<metal>(albedo, fuzz);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                } 
                else {
                    // glass
                    sphere_material = std::make_shared<dielectric>(1.5);
                    world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = std::make_shared<dielectric>(1.5);
    world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));
    auto material2 = std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));
    auto material3 = std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));
    
    return world;
}

surface_list scene1() {
    surface_list world;
    
    auto material_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = std::make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left   = std::make_shared<dielectric>(1.5);
    auto material_right  = std::make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);
    
    world.add(std::make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(std::make_shared<sphere>(point3( 0.0, 0.0, -1.0), 0.5, material_center));
    world.add(std::make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(std::make_shared<sphere>(point3( 1.0, 0.0, -1.0), 0.5, material_right));

    return world;
}

int main() {
    
    // Image
    const double aspect_ratio   = 16.0/9.0; //3.0 / 2.0;        // 定义2D渲染图像的默认比例是16:9. 也就是宽是16, 高9. 也即一行所包含的像素点和一列所包含的像素点比例为16比9.
    const int image_width       = 400; //1200;             // 定义图像上一行包含的像素点的个数.
    const int image_height      = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 100; //500;              // 抗锯齿功能开启, 对一个pixel采样100个样本点. 
    const int max_depth         = 50;               // 反射的最大次数. 也就是光线追踪的最大迭代次数

    // world. 
    //surface_list world = random_scene();        // world是一个surface_list, 包含所有出现在3D场景中的object.
    surface_list world = scene1();

    /*
    // Define material object. RGB -> red & green & blue. 
    // 折射率是一个RGB向量, 对每个基颜色反射率不同, 折射率分量每个值都在[0.0, 1.0]之间, 值越大对于这一基颜色反射能力越强.

    // world->background.
    // 不同的材质, 分配不同的折射率. 这一漫反射材质更能反射黄光, 红+绿 = 黄.
    auto material_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
    world.add(std::make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));        // 这一方式定义的球实则是表示一个地面background.
    
    // world->spheres. 三个球.
    //auto material_center = std::make_shared<lambertian>(color(0.7, 0.3, 0.3));      // 这一漫反射材质更能反射红光.
    //auto material_center = std::make_shared<dielectric>(1.5);           // 电介质材质, 折射率ir = 1.5. typically air = 1.0, glass = 1.3–1.7, diamond =2.4
    auto material_center = std::make_shared<lambertian>(color(0.1, 0.2, 0.5));      // 这一漫反射材质更能反射蓝光.
    world.add(std::make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
    
    // 这一金属材质均匀反射光. 所以其镜面反射能让这一球体把背景原封不动的反射出来. 模糊反射系数为0.3, 模糊度低
    // auto material_left   = std::make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);     
    auto material_left   = std::make_shared<dielectric>(1.5);           // 电介质材质, 折射率ir = 1.5. typically air = 1.0, glass = 1.3–1.7, diamond =2.4
    world.add(std::make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(std::make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.45, material_left));      // 半径为负, 表面法向量向内指向, 一正球一"负"球实现空心玻璃球效果.

    // 金属材质更能反射红光和绿光, 红+绿=黄, 所以这一表面会偏黄绿. 模糊反射系数为1.0, 模糊度强.
    // auto material_right  = std::make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);         
    auto material_right = std::make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);        // 模糊系数为0.0, 无模糊. 精确镜面反射.  
    world.add(std::make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));
    */

    // camera.
    //camera cam(point3(-2.0,2.0,1.0), point3(0.0,0.0,-1.0), vec3(0.0,1.0,0.0), 90.0, aspect_ratio);
    //camera cam(point3(-2.0,2.0,1.0), point3(0.0,0.0,-1.0), vec3(0.0,1.0,0.0), 20.0, aspect_ratio);      // 缩小视角, 可视场景范围变小, 视野变深, 越能看清楚物体纹理
    camera cam(point3(0.0,0.0,0.0), point3(0.0,0.0,-1.0), vec3(0.0,1.0,0.0), 90.0, aspect_ratio, 0.0, 1.0);
    
    /*
    point3 lookfrom(13.0, 2.0, 3.0);         // 右手坐标系, 这个视角就离球远一些, 从上方俯视球.
    point3 lookat(0.0, 0.0, 0.0);
    vec3 vup(0.0, 1.0, 0.0);
    double aov = 20.0;
    double aperture = 0.1;
    double focus_dist = 10.0; //(lookfrom - lookat).length();               // 让lookat点就为成像平面中心.

    camera cam(lookfrom, lookat, vup, aov, aspect_ratio, aperture, focus_dist);
    */

    // Render
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    /*  
        计算机图形学做的事情和计算机视觉刚好相反. 计算机图形学是给定3D空间场景生成2D图片, 而计算机图形学是给定2D图片, 分析2D图片所包含的3D物体信息.
        下图的w-h 2D平面坐标空间, 是我们要生成的渲染图像image的平面空间.
            1. 渲染图像宽为width, 表示的是一列有width个像素点; 高为height, 表示的是一行有height个像素点. 渲染图像总共包含 包含width*height个像素. 
            2. 每个像素点坐落于边长为1单位像素长度(两个像素点的距离)的小正方形的中心. 这个小正方形所覆盖的区域都会使用像素点的颜色来着色.
            3. 渲染图像只表示我们想要渲染的像素点总数有多少个, 并不表示光线追踪程序中成像平面image_plane的大小. image_plane的大小是有摄像机视角, 聚焦距离和屏幕比例共同决定的.
               在光线追踪器程序中, 我们需要把渲染图像空间的像素点的坐标位置(i,j)正确映射成光线追踪程序中在3D世界坐标系空间下成像平面上对应的正确位置(u,v).
               如果成像平面大, 这通常意味着摄像机视角范围大, 这意味着被正确映射后的两个相邻像素点之间的在u-v平面的间隔距离就会大, 这样好处是渲染图像显示3D场景空间范围大, 但图像容易不清晰.
               如果成像平面小, 这通常意味着摄像机视角范围小, 这意味着被正确映射后的两个相邻像素点之间的在u-v平面的间隔距离就会小, 这样好处是渲染图像会比较清晰, 但显示的3D场景空间范围会小.              
            4. 渲染图像的遍历的默认顺序是从高到低, 从左到右, 所以index j从iheight的遍历

        ^ height h-轴       j = image_height - 1 to 0 with stepsize = -1.
        |
        |
        | (0, image_h-1)
        |--------------------------------
        |                               |
        |                               |
        |            image              |
        |                               |
        |    pixels = width x height    |
        |                               |           i =  0 to image_width - 1 with stepsize = 1.
        |                               |
        ---------------------------------------------> width w-轴
     (0,0)                              (image_width - 1, 0)
     */
    for(int j = image_height-1; j >= 0; --j) {
        std::cerr << "\rScanlines remaing: " << j << ' ' << std::flush;
        for(int i = 0; i < image_width; ++i) {
            color pixel_color(0.0, 0.0, 0.0);
            /*  抗锯齿, antialiasing.
                这里我们使用随机采样抗锯齿, 在w-h平面上以像素点为中心的边长为1个单位像素长度的正方形邻域内随机采样着色位置.
                然后把这样采样的着色位置映射到u-v成像平面, 以此在u-v成像平面我们也就在一个特定邻域内随机取到了像素点在成像平面的坐标位置.
                然后对每一个这样在邻域内随机取得的像素点坐标位置进行执行光线追踪算法算出颜色, 对所有这样的采样像素位置的颜色值进行平均化, 最终就是该像素点的值.

                随机采样是很简单的抗锯齿技术, 还有更高级一些的分层随机采样抗锯齿技术, 对像素点选取的采样邻域进行扰动. 
                此时虽然初始仍以像素点为中心选取邻域, 但是引入的随机扰动会使得选取的邻域的中心相对于像素点出现随机偏离. */
            for(int k = 0; k < samples_per_pixel; ++k) {
                /* 这里用了一个很巧妙的方法, 当确定了渲染图像像素点的空间坐标值(i,j)之后, 并没有直接把两个整数(i,j)传给摄像机让摄像机来转换映射.
                   而是先除以对应的image_width-1和image_height-1, 得到的是这一像素点与渲染图像空间关于两个坐标轴的分量比例值.
                   对于h轴分量j, 得到了比例分量s, s在[0,1]之间; 对于w轴分量i, 得到了比例分量t, t在[0,1]之间.
                   然后把这两个比例分量传给摄像机, 这样做可以很明显简化摄像机内部把像素点在渲染图像空间坐标值转换到u-v平面上正确世界坐标值的计算. 
                   我们只需要在摄像机内部存储好:
                                    1. 摄像机位置lookfrom, 成像平面的左下角顶点坐标lower_left_vertex;
                                    2. 成像平面在u-v空间上, u轴的长度为成像平面宽度的基向量horizontal, v轴的长度为成像平面高度的基向量vertical.
                   然后使用从h-w平面得到的像素点比例分量s和t, 就能立即确定像素点映射在成像平面的正确位置(即正确的世界坐标值), 或者确定从视点发出的指向这一像素点在成像平面位置的射线的方向:
                                    loc     = lower_left_vertex + s*horizontal + t*vertical
                                    ray_dir = (lower_left_vertex - cam_origin) + s*horizontal + t*vertical */
                double s = (i + random_double()) / (image_width - 1);
                double t = (j + random_double()) / (image_height - 1);
                // 以视点射向成像平面最左下角顶点的射线为base, 通过add在horizontal所代表的的u轴基向量和vertical所代表的v轴基向量的增量offset, 来确定正确的穿过成像平面"像素点"的射线.
                // base_dir     = lower_left_corner - origin        => 表示的是以视点射向成像平面最左下角顶点的射线
                // x_dir_offset = u*horizontal; y_dir_offset = v*vertical;
                ray r = cam.get_ray(s, t);          // 摄像机这个对象负责生成光线. 
                // 找到第一个与3D场景物体列表的相交点, 然后计算像素值!
                pixel_color += ray_color(r, world, max_depth);
            }
            // use write_color function to print out the color value in [0, 255].
            write_color(std::cout, pixel_color, samples_per_pixel);     // IO操作是一个很耗时的操作, 放入采样loop中输入一下子耗时就大了可能1-2个数量级.

            // 使用".\ppmImageText.exe > image.ppm" command把输出变成ppm格式图片. 注意用右箭头">", 这个是关键.
        }
    }

    std::cerr << "\nDone.\n";

    return 0;
}

/*
    光线追踪器程序框架.

    1. 确定渲染图像的大小. 渲染图像大小是以像素为单位. 1200*800的渲染图像表示的是1200行每一行有800个像素点, 800列每一列有1200个像素点.
    2. 建立渲染图像w-h平面上像素点坐标位置(i,j)到摄像机成像平面坐标位置(u,v)的映射, 使用的trick是使用像素点的比例分量s和t(像素点的比例分量无论是在渲染图像还是成像平面都是相同的).
    
    3. 定义向量vector类, 并重载vector运算所有需要的运算符.
    4. 开始定义utility类并贯穿整个程序实现中, 一直持续添加代码直到光线追踪器彻底成型. utility类包含所有我们需要的dirty-hanlder function. 包括生成各种球内/球面/圆面随机点或随机向量.

    4. 定义射线ray类. ray类是ray tracer的一个核心类, 使用vector类来定义. 一条射线由一个起点和方向确定.
    5. 定义摄像机camera类. camera类是ray tracer的一个核心类. 光线追踪的所有穿透成像平面的射线都是从camera中发出的.
       定义的camera类是一个可以在空间中自由放置位置, 自由定义摄像方向, 自由定义视角大小, 自由决定成像平面屏幕比例和高度, 自由决定聚焦平面离摄像机距离, 自由定义光圈大小的这样一个摄像机类.

    6. 定义surface抽象基类, 任意一个可以被射线击中的3D空间物体都需定义成surface类的子类, 并实现有关射线击中的hit函数.
       virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const  = 0;    
    7. 定义hit_record结构体, 它包含函数hit()返回true时射线与物体交点的空间坐标值, 射线的交点参数t, 交点法向量, 交点表面是否为正向(法向量指向空气方向为正), 交点表面材质, 等数据.
    8. 定义surface_list类, 为surface的派生类. 这是一个可以包含任意多个surface子类的类似于容器类. 给定一个3D场景, 这个3D场景一定是被一个个的物体所组成的(背景background也是一个"物体").
       由此就能用surface_list类对象类刻画这一3D场景世界.
    9. 定义sphere类, 为surface的子类. 自如其名, 刻画圆球这样一个3D物体.

    10. 定义material抽象基类, 任意一种可以表示物体表面的材质都需要定义成material类的子类, 并在其中实现对于光的散射scatter(光的散射包含漫反射, 镜面反射和折射)函数.
        virtual bool scatter(const ray& r, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
        10.1 定义Lambert材质类, 这种材质只有漫反射. 包含数据成员albeda, 表示对光线的RGB反照率, 是反射光总能量和入射光总能量的比值, 表示的是材质吸收光能量的能力. 
                                                  albeda是一个该材质总体的对于单位强度入射光能够反射/吸收多少能量的一个均值体现.
        10.2 定义metal金属材质, 这种材质只有镜面反射. 包含数据成员albeda, 表示对光线的RGB反照率, 是反射光总能量和入射光总能量的比值, 表示的是材质吸收光能量的能力.
                                                    albeda是一个该材质总体的对于单位强度入射光能够反射/吸收多少能量的一个均值体现
        10.3 定义dielectric电介质材质, 这种材质具有折射和全内反射. 包含数据成员ir, 表示材质对光线的折射率, 折射率越大折射光线的能力越强, 折射光线与法线负方向夹角越小.
                                                                 包含计算reflectance反射率函数, reflectance反射率, 描述的是反射和折射的光强比重. 
                                                                 用来衡量物质反射能力的量, 定义为物体表面反射能量与到达物体表面入射能量的比率. 反射率是波长的函数, 且一般为正.
                                                                 反射率为R意味着1单位的入射光能力将会有R数量反射光能力, 1-R数量的折射光能量.
        10.4 我们可以暂时把albeda理解为(漫/镜面)反射情况下材质对入射光辐射能量的吸收能力即可, albeda越小材质吸收光辐射能量越强.
                    暂时把reflectance理解为折射情况下, 材质对于入射光能量的折射能力, reflectance越小物质所分化的折射光能量越强.
    
    11. 实现anti-aliasing抗锯齿技术. 在w-h平面上以像素点为中心的边长为1个单位像素长度的正方形邻域内随机采样着色位置.
        然后把这样采样的着色位置映射到u-v成像平面, 以此在u-v成像平面我们也就在一个特定邻域内随机取到了像素点在成像平面的坐标位置.
        然后对每一个这样在邻域内随机取得的像素点坐标位置进行执行光线追踪算法算出颜色, 对所有这样的采样像素位置的颜色值进行平均化, 最终就是该像素点的值.
    
    12. 实现摄像机的景深.
        a. 以摄像机视点为中心与w轴垂直的平面定义一个圆盘用来模拟透镜/光圈. 在这个光圈上随机取视点位置往像素点发射射线, 如此就模拟出汇聚到像素点的光线来自于不同的物体空间点(除了焦平面).
        b. 光圈发出的随机光线会汇聚在成像平面的像素点上, 那自然成像平面就是焦平面. 移动成像平面相对于摄像机的位置, 就是在移动聚焦的平面. 成像平面前后仍然能在平面上清晰成像的距离就是景深.
           光圈越大, 那么从随机视点到像素点方向的角度(水平夹角)就能越大, 景深就越窄; 光圈越小, 那么从随机视点到像素点方向的角度就越小, 景深越广.
        c. 因为成像平面就是聚焦平面, 所以没有焦距概念. 直接使用focus_dist聚焦距离当做焦距, 并和aov伙同视角公式就可以计算出成像平面的高度.
           half_h = tan(aov) * focus_dist. 改变focus_dist就是在改变模拟光圈圆盘的焦距.

    13. 实现recurive shader着色程序. 当射线找到与物体的交点之后, 我们需要递归的再次从交点发出射线以便知晓反射/折射射线最终去向何处.
        每反射/折射一次, 收到的光强度都要以材质表面反射损失率或折射损失率来减弱.
*/                      
/*
    Difference between albedo反照率 vs reflectivity反射率.

    Rita Freuder's Answer: I checked up on a definition of albedo and found this one helpful, along with its examples:
    Albedo is: 
                The fraction of the total light striking a surface that gets reflected from that surface. 
                An object that has a high albedo (near 1) is very bright; an object that has a low albedo (near 0) is dark. 
                The Earth's albedo is about 0.37. The Moon's is about 0.12.

    1. Reflected shortwave radiation is measured in Energy units. According to the "About this data" for the NASA NEO dataset of reflected shortwave radiation,
       The colors in the map show the amount of shortwave energy (in Watts per square meter) that was reflected by the Earth system for the given time period(s).
    2. The percent of reflection is probably directly proportional to the reflected energy. In the EM spectrum, the shorter the wave length, the higher the energy of the light. 
    3. So, you could have very high reflectance of low energy "light" - maybe even near 100 % which would be a 1. 
       But since the light striking the surface and reflecting is low energy, the "number" that represents the energy will also be lower. 
    4. Albedo seems to be the reflective property of an object - it could be applied to planets, or landcover. 
       Reflected shortwave radiation is the measure of the energy reflected from planet. 
    5. Reflected shortwave radiation changes daily and will depend in part on what energy of light is striking the surface, and perhaps on cloud-free conditions. 
       Albedo assumes the conditions are cloud free when averaging the fraction light reflected from all the different surface textures on Earth (oceans, deserts, forests, etc).
    6. The range of values for the reflected short-wave radiation can be seen when you graph it using the ICE tool - maybe to draw a transect. 
       The range of values for albedo is always 0-1.
*/

/*
    三棱镜折射.  入射光线首先被折射进入介质内部, 然后再从介质内部折射出. 两次折射后进入空气中的光线方向恰好和入射方向相反.
                对于具有电介质材质的球来说, 具有同三棱镜一样的折射效果. 所以球表面会显示出背景景色的倒影.
                                                /\
                                               /  \
                                              /    \
                                            >/----->\
                                         /  /        \   \
                                    /      /          \       \ 
                               /          /            \           \
                          /              /______________\               \
                                                                           V
*/