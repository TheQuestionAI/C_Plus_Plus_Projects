#ifndef COLOR_H
#define COLOR_H

#include "utility.h"

#include <iostream>

// Using our vec3 class, we'll create a utility function to write a single pixel's color out to the standard output stream.
// 增加简单的抗锯齿技术, 也就是对每个像素在它的小正方形grid内进行采样. 对所有采样点求均值求得像素值.
void write_color(std::ostream& out, const color& pixel_color, const int samples_per_pixel) {
    // 转化成[0,255]的标准RGB值输出.
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Divide the color by the number of samples and gamma-correct for gamma=2.0. 对采样累加值求均值得到最终像素值, 然后再进行gamma校正.
    auto scale = 1.0 / samples_per_pixel;
    r = std::sqrt(scale * r);
    g = std::sqrt(scale * g);
    b = std::sqrt(scale * b);

    // Write the translated [0,255] value of each color component.
    // 注意这里最大值是clamp的最大值是0.999, 这里就是computer computation的小trick. 
    // 这样我们取外围最大值256, 就可以保证我们乘以clamp值可以取整到[0,255]之间.
    out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

#endif