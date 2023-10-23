#ifndef __TOUCH_H_
#define __TOUCH_H_

#include <string>
using namespace std;

typedef struct Postion
{
    int x;
    int y;
} Pos;

class Touch
{
private:
    enum BoardType
    {
        Black,
        Blue,
    } m_type;
    string m_path;

private: // static
    static Touch *interface;

private:
    Touch(BoardType type = Black, string dev = "/dev/input/event0");
    Touch(BoardType type, char *dev);

public:
    // 阻止拷贝构造函数和赋值运算符
    Touch(const Touch &) = delete;
    Touch &operator=(const Touch &) = delete;
    // 为了阻止编译器自动生成拷贝构造函数和赋值运算符，我们将它们声明为删除函数（deleted function）
    // 这样可以确保单例对象不会被复制或赋值。

public:
    static Touch *self(BoardType type, char *dev);
    static Touch *self(BoardType type = Black, string dev = "/dev/input/event0");

public:
    void info() const;
    Pos wait();
    void wait(int &rx, int &ry);
    ~Touch();
};

#endif