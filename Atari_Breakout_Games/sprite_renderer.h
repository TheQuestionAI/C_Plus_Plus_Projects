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
* ����ͼ��Ⱦ��
* ����ͼsprite: ����ͼʵ���Ͼ���һ��λ��3D�ռ��е�2Dͼ��(�������������)��һЩ��3D���������ڶ�λ��һͼ������ݼ���.
*				����һ������ͼ�������������:
*					1. tuxture object: ����һ��2D����ͼ����������.
*					2. position: ����ͼ���ڿռ��λ��, �Ծ���ͼ���ϽǶ���Ϊ��λ.
*					3. size: ����ͼ�Ĵ�С.
*					4. rotation: ����ͼ�Ƿ���ת, ��ת�Ķ���.
*					5. color: ����һ��colorϵ������������ͼ�����ɫ. �����������ͼ����һ���Ҷ�ͼ��, ��ô������ͨ��color�������䲻һ����ɫ��.
*
* ����ͼ��Ⱦ��: ר�����ڻ��ƾ���ͼ����, ֻ�ṩһ������, ������ͼ���ƺ���DrawSprite(), ��һְ��ģʽ.
*				����һ������ͼ��Ⱦ���������������:
*				1. shader: ��Ȼ��Ҫһ����ɫ���������, ���ܽ���ͼ����ƹ���.
*				2. square: �Ծ���ͼ�Ļ���, �䱾�ʾ��Ƕ�һ����Ϊ��ʵ���������ν��и��־���任����ɫ�ӳֺ�Ľ��. ���ʵ����Ԫ/ʵ��ģʽ, ��Ҫһ��ʵ��������.
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
    // ��ʵ������������������ݴ洢��GPU buffer object��.
    // ������Ҫ��������ͼ��ӳ��, ���������ȱ���Ҫ���嶥������, ����һ��2D����ͼ��, ����ֻ��Ҫ���峤���ε��ĸ������Լ���Ӧ����������, Ȼ��������Բ�ֵ�Ϳ��Ի������ͼ�����������������.
    // sprite uses the same original square shape. ����ʹ�������ģʽ�е�ʵ��ģʽ/��Ԫģʽ. 
    float square[] = {            // ������ʵ������������½Ƕ���������ԭ���(0,0)��λ������. �������ĸ������Ӧ�ĸ���ͬ����������.
        // pos      // tex coord
        0.0f, 1.0f, 0.0f, 1.0f,     // ֮��Ҫ������ͨ��model����Ի��������ν���ʵ����, ת����һ����δ��ɫǰ�ľ���ͼ. 
        1.0f, 0.0f, 1.0f, 0.0f,     // ���ڲ�δִ��projection matrix, ��������û��flip�ɴ��ڿռ�����, �������������½Ƕ���Ϊê��ê��ԭ��(0,0)λ��.
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
    // ��׼�仯˳��: ���ȷ���, ֮����ת, ���ƽ��.
    
    // ��Ϊ2D game, �Ǻ���Ȼ���ǲ���Ҫz�����(Ϊ0), �������ǿ��Լ򵥵İ�����ռ��µ�x-y��һ����ƽ��(+x, +y)����Ϊ��Ϸƽ��, Ȼ�����ж�������궼��[0, WIDTH] X [0, HEIGHT]��2D����ռ���Ϸƽ����.
    // ���, ����Ҫ���ľ���ʹ��model����ֱ�ӿ��԰Ѿ���ͼ��local object space�µ�λ��ԭ��Ϊê���λ��, �任��2D����ռ�����[0, WIDTH] X [0, HEIGHT] X 0 Ϊ�����λ����.
    // view transformation�Ϳ���ʡ��, ��Ϊ2D game����Ŀ����������[0, WIDTH] X [0, HEIGHT]��һ2D��Ϸ�ռ�. 
    // ����ֻ���ڶ�����ɫ������ʹ��model�任��local position�任��2D game space, Ȼ������ͶӰ�任��2D��Ϸ�ռ�����任��NDC�ռ����꼴��.
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    glm::mat4 model = glm::mat4(1.0f);      // ��ʼ��model����.

    // translate third.
    model = glm::translate(model, glm::vec3(position, 0.0f));
    
    // rotate second. 
    // �Ծ���ͼ������Ϊê��(���������½Ƕ���)������ת, ��������ƽ��ʹ�þ���ͼ���ĵִ�����ԭ��λ��, Ȼ����ת, ���Ѿ���ͼ����ƽ�ƻص�ԭ��λ��.
    model = glm::translate(model, glm::vec3(0.5 * size.x, 0.5 * size.y, 0.0f));         // third.
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));    // second.
    model = glm::translate(model, glm::vec3(-0.5 * size.x, -0.5 * size.y, 0.0f));       // first.

    // scale first.
    model = glm::scale(model, glm::vec3(size, 1.0f));
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    // ȷ��GLSL�б�����ȡ����������һ�µ�.
    this->shader->Use();
    this->shader->SetInteger("sprite", texture->UnitID());
    this->shader->SetMat4x4("model", model);
    this->shader->SetVector3f("color", color);

    this->vao.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);       // ����OpenGL����������ƾ���ͼ.
    this->vao.UnBind();
}

#endif // !SPRITE_RENDERER_H

