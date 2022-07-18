#include <iostream>

int main() {

    // Image
    const int imageWidth  = 256;
    const int imageHeight = 256;

    // Render
    std::cout << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";

    /*
        image是使用2D array来表示, 但是image的width和height与2D array的存储(x,y)index刚好相反.
        x => 表示的是height, x++或者x--表示image的height方向的像素移动.
        y => 表示的是width, y++或者y--表示image的width方向的像素移动.
        给定一个array cell A[i][j] => 表示的是image上第i+1行第j+1列的像素点.

        还有一个2D array表示的坐标系是从origin(0,0)出发, 然后x轴从左向右迭代, y轴从下到上迭代.
        ^
        |
        |
        |
        |
        |
        | 
        --------------------------------->
        而对于image, 我们则是width方向继续从左到右迭代, 但是height方向是从上到下迭代.
        --------------------------------->
        |
        |
        |
        |
        |
        | 
        v
        由此对于image的像素迭代会是
            1. outer loop是height, 从2D array的y轴的最大列值(imageHeight-1)出发, 迭代到y=0处.
            2. inner loop是width, 从2D array的x轴x=0出发, 迭代到最大值处(imageWidth-1).
     */
    for(int j = imageHeight-1; j >= 0; --j) {
        std::cerr << "\rScanlines remaing: " << j << ' ' << std::flush;
        for(int i = 0; i < imageWidth; ++i) {
            // Red goes from fully off (black) to fully on (bright red) from left to right. 
            // Green goes from black at the bottom to fully on at the top. 
            // Red and green together make yellow so we should expect the upper right corner to be yellow.
            auto r = static_cast<double>(i) / (imageWidth - 1);
            auto g = static_cast<double>(j) / (imageHeight - 1);
            auto b = 0.25;      // fix blue color value.
            
            // conver to integer in [0,255].
            int ir = static_cast<int>(255.999 * r);
            int ig = static_cast<int>(255.999 * g);
            int ib = static_cast<int>(255.999 * b);

            // 使用".\ppmImageText.exe > image.ppm" command把输出变成ppm格式图片. 注意用右箭头">", 这个是关键.
            std::cout << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }

    std::cerr << "\nDone.\n";

    return 0;
}