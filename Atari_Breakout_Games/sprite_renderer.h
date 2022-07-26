#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

#include <iostream>
#include <memory>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "gl_object.h"
#include "shader.h"
#include "texture.h"

/*
* 精灵图渲染器
* 精灵图sprite: 精灵图实际上就是一个位于3D空间中的2D图像(采用纹理对象获得)和一些在3D世界中用于定位这一图像的数据集合.
*				定义一个精灵图需包含的属性有:
*					1. tuxture object: 包含一张2D纹理图像的纹理对象.
*					2. position: 精灵图所在空间点位置, 以精灵图左上角顶点为定位.
*					3. size: 精灵图的大小.
*					4. rotation: 精灵图是否旋转, 旋转的度数.
*					5. color: 利用一个color系数来调节纹理图像的颜色. 例如如果纹理图像是一个灰度图像, 那么很容易通过color来调节其不一样的色彩.
*
* 精灵图渲染器: 专门用于绘制精灵图的类, 只提供一个函数, 即精灵图绘制函数DrawSprite(), 单一职责模式.
*				定义一个精灵图渲染器需包含的属性有:
*				1. shader: 必然需要一个着色器程序对象, 才能进行图像绘制工作.
*				2. square: 对精灵图的绘制, 其本质就是对一个作为基实例的正方形进行各种矩阵变换和颜色加持后的结果. 因此实行享元/实例模式, 需要一个实例正方形.
*/
class SpriteRenderer        // checked, no need to check again!
{   
public:
    SpriteRenderer(std::shared_ptr<Shader> sd);
public:
    void DrawSprite(std::shared_ptr<Texture2D> texture, glm::vec2 position, glm::vec2 size, float rotation = 0.0f, glm::vec3 color = glm::vec3(1.0f));
private:
    std::shared_ptr<Shader> shader;
    VAO vao;
    VBO vbo;
};

SpriteRenderer::SpriteRenderer(std::shared_ptr<Shader> sd) : shader{ sd }
{
    // 把实例顶点和纹理坐标数据存储在GPU buffer object中.
    // 我们想要进行纹理图像映射, 那我们首先必须要定义顶点数据, 对于一个2D纹理图像, 我们只需要定义长方形的四个顶点以及相应的纹理坐标, 然后进行线性插值就可以获得纹理图像下所有坐标的纹理.
    // sprite uses the same original square shape. 这里使用了设计模式中的实例模式/享元模式. 
    float square[] = {            // 真正的实例就是这个左下角顶点在坐标原点的(0,0)单位正方形. 正方形四个顶点对应四个不同的纹理坐标.
        // pos      // tex coord
        0.0f, 1.0f, 0.0f, 1.0f,     // 之后要做的是通过model矩阵对基础正方形进行实例化, 转化成一个个未着色前的精灵图. 
        1.0f, 0.0f, 1.0f, 0.0f,     // 由于并未执行projection matrix, 所以坐标没有flip成窗口空间坐标, 仍以正方形左下角顶点为锚点锚在原点(0,0)位置.
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    this->vao.Bind();
    this->vbo.Bind();
    this->vbo.LoadVertexData(sizeof(square), square);
    this->vbo.ConfigureVertexAttribute(0, 4, GL_FLOAT, 4 * sizeof(float), 0);
    this->vbo.UnBind();
    this->vao.UnBind();
}

void SpriteRenderer::DrawSprite(std::shared_ptr<Texture2D> texture, glm::vec2 position, glm::vec2 size, float rotation, glm::vec3 color)
{
    // standard transformation order v' = translate * rotation * scale * v.
    // 标准变化顺序: 最先放缩, 之后旋转, 最后平移.
    
    // 作为2D game, 那很显然我们不需要z轴分量(为0), 所以我们可以简单的把世界空间下的x-y第一象限平面(+x, +y)定义为游戏平面, 然后所有顶点的坐标都在[0, WIDTH] X [0, HEIGHT]的2D世界空间游戏平面上.
    // 如此, 我们要做的就是使用model矩阵直接可以把精灵图在local object space下的位于原点为锚点的位置, 变换到2D世界空间中以[0, WIDTH] X [0, HEIGHT] X 0 为区域的位置上.
    // view transformation就可以省略, 因为2D game相机的可视区域就是[0, WIDTH] X [0, HEIGHT]这一2D游戏空间. 
    // 所以只需在顶点着色器中先使用model变换把local position变换到2D game space, 然后经正交投影变换把2D游戏空间坐标变换到NDC空间坐标即可.
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    glm::mat4 model = glm::mat4(1.0f);      // 初始化model矩阵.

    // translate third.
    model = glm::translate(model, glm::vec3(position, 0.0f));
    
    // rotate second. 
    // 对精灵图以中心为锚点(而不是左下角顶点)进行旋转, 先往坐下平移使得精灵图中心抵达坐标原点位置, 然后旋转, 最后把精灵图中心平移回到原来位置.
    model = glm::translate(model, glm::vec3(0.5 * size.x, 0.5 * size.y, 0.0f));         // third.
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));    // second.
    model = glm::translate(model, glm::vec3(-0.5 * size.x, -0.5 * size.y, 0.0f));       // first.

    // scale first.
    model = glm::scale(model, glm::vec3(size, 1.0f));
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    // 确保GLSL中变量的取名与这里是一致的.
    this->shader->Use();
    this->shader->SetInteger("sprite", texture->UnitID());
    this->shader->SetMat4x4("model", model);
    this->shader->SetVector3f("color", color);

    this->vao.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);       // 调用OpenGL绘制命令绘制精灵图.
    this->vao.UnBind();
}

#endif // !SPRITE_RENDERER_H

