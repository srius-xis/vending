#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include <cstring>
#include <linux/input.h>

#include "touch.h"

Touch *Touch::interface = NULL;

Touch::Touch(BoardType type, string dev)
    : m_path(dev), m_type(type)
{
    // info();
}

Touch::Touch(BoardType type, char *dev)
    : m_path(dev), m_type(type)
{
    // info();
}

Touch *Touch::self(BoardType type, char *dev)
{
    if (!interface)
        interface = new Touch(type, dev);
    else
        cout << "already load device\n";

    return interface;
}

Touch *Touch::self(BoardType type, string dev)
{
    if (!interface)
        interface = new Touch(type, dev);

    return interface;
}

Pos Touch::wait()
{
    int toufd = open(m_path.c_str(), O_RDWR);
    if (toufd == -1)
    {
        cout << "open " << m_path << " faild\n";
        return (Pos){-1, -1};
    }

    struct input_event info;
    memset(&info, 0, sizeof(info));
    Pos pos, tmp;
    memset(&pos, 0, sizeof(pos));
    while (1)
    {
        read(toufd, &info, sizeof(info));

        // 判断当前发生的是触摸屏事件,接着判断触发的是X轴事件
        if (info.type == EV_ABS && info.code == ABS_X)
        {
            tmp.x = info.value;
        }
        // 判断当前发生的是触摸屏事件,接着判断触发的是Y轴事件
        if (info.type == EV_ABS && info.code == ABS_Y)
        {
            tmp.y = info.value;
        }

        // 松开的时候，才进行打印  type:1 code:330 value:0
        if (info.type == EV_KEY && info.code == BTN_TOUCH && info.value == 0)
        {

            // 如果你的板子是黑色的，那么需要进行坐标转换(1024,600) ---(800,480)
            if (m_type == Black)
            {
                pos = (Pos){(int)((tmp.x * 1.0) / 1024 * 800), (int)((tmp.y * 1.0) / 600 * 480)};
            }

            std::cout << "x:" << tmp.x << " y:" << tmp.y << std::endl;

            break;
        }
    }

    // 5）关闭触摸屏文件
    close(toufd);
    return pos;
}

void Touch::info() const
{
    cout << "path: " << m_path << " board type: " << (m_type == Black ? "Black" : "Blue") << endl;
}

void Touch::wait(int &rx, int &ry)
{
    int toufd = open("/dev/input/event0", O_RDWR);
    if (toufd == -1)
    {
        printf("open /dev/input/event0 error\n");
        return;
    }

    struct input_event info;
    memset(&info, 0, sizeof(info));
    while (1)
    {

        // 3）读取触摸屏数据
        read(toufd, &info, sizeof(struct input_event));

        // 判断当前发生的是触摸屏事件,接着判断触发的是X轴事件
        if (info.type == EV_ABS && info.code == ABS_X)
        {
            rx = info.value;
        }
        // 判断当前发生的是触摸屏事件,接着判断触发的是Y轴事件
        if (info.type == EV_ABS && info.code == ABS_Y)
        {
            ry = info.value;
        }

        // 松开的时候，才进行打印  type:1 code:330 value:0
        if (info.type == EV_KEY && info.code == BTN_TOUCH && info.value == 0)
        {

            // 如果你的板子是黑色的，那么需要进行坐标转换(1024,600) ---(800,480)
            if (m_type == Black)
            {
                rx = (rx * 1.0) / 1024 * 800;
                ry = (ry * 1.0) / 600 * 480;
            }

            std::cout << "x:" << rx << " y:" << ry << std::endl;

            break;
        }
    }
    // 5）关闭触摸屏文件
    close(toufd);
}
