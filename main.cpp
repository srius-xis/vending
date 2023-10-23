#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <cerrno>
#include <cstring>

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "./cls_lcd/lcd.h"
#include "./cls_product/product.h"
#include "./cls_tou/touch.h"
#include "./cls_lcd/cls_bmp/bmp.h"
#include "./cls_json/cJSON.h"
#include "./cls_lcd/cls_wb/wordbank.h"
#include "./cls_tcp/tcpsock.h"
// #include "./cls_tcp/tcpser.h"

#ifndef debug
#define debug printf("\nfile is: <<%s>>, function is: {%s}, line is: [%d]\n", __FILE__, __FUNCTION__, __LINE__);
#endif

using namespace std;

#define pass \
    {        \
    }

Touch *tou = Touch::self();
WordBank *wb = WordBank::self();

int max_page = 0;

sem_t sem;

bool ex = false;
bool ex_count = false;

vector<Product> head;

bool cmp(Product &ra, Product &rb)
{
    return ra.sales() > rb.sales();
}

void product_json_parse()
{
    std::FILE *fp = fopen("product.json", "rb");
    if (NULL == fp)
    {
        cout << "open json file faild: " << strerror(errno) << "\n";
        return;
    }

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buf = new char[len + 1];
    memset(buf, 0, len + 1);
    fread(buf, 1, len, fp);
    fclose(fp);

    cJSON *root = cJSON_Parse(buf);
    cJSON *json = cJSON_GetObjectItem(root, "product");
    for (int i = 0; i < cJSON_GetArraySize(json); i++)
    {

        max_page = cJSON_GetArraySize(json) / 10;
        cJSON *pro = cJSON_GetArrayItem(json, i);

        head.push_back((Product(
            cJSON_GetStringValue(cJSON_GetObjectItem(pro, "name")),
            cJSON_GetStringValue(cJSON_GetObjectItem(pro, "path")),
            atoi(cJSON_GetStringValue(cJSON_GetObjectItem(pro, "stock"))),
            cJSON_GetStringValue(cJSON_GetObjectItem(pro, "price")),
            atoi(cJSON_GetStringValue(cJSON_GetObjectItem(pro, "sales"))))));

        // printf("%-11s\t%-16s%-3d%-.2f \n", head[i].name().c_str(), head[i].path().c_str(), head[i].stock(), head[i].price());
    }
    cJSON_Delete(root);

    delete[] buf;

    sort(head.begin(), head.end(), cmp);
}

void cart(int index = -1, bool add = true)
{
    if (-1 == index)
    {
        for (int i = 0; i < 30; i++)
        {
            head[i].count = 0;
        }
        return;
    }
    else if (0 <= index && index < 30)
    {
        if (add)
        {
            if (head[index].count < head[index].stock())
                head[index].count++;
            else
                cout << "The maximum inventory value has been reached\n";
        }
        else
        {
            if (0 < head[index].count)
                head[index].count--;
            else
                cout << "The purchase has been cancelled\n";
        }
    }
}

void exit_loop()
{
    cart(-1);
}

void show_list()
{
    int i = 0;
    for (auto it = head.begin(); it != head.end(); it++)
    {
        printf("%d ", ++i);
        printf("%-11s\t%-16s%-3d%3s\n", it->name().c_str(), it->path().c_str(), it->stock(), it->price().c_str());
    }
}

int cart_event()
{
    int x = -1, y = -1;
    tou->wait(x, y);

    if (425 < y)
    {
        if (80 < x && x <= 80 + 80)
            return 21; // exit();
        else if (80 + 80 + 60 < x && x <= 80 + 80 * 2 + 60 * 1)
            return 22; // clear cart
        else if (80 + 80 * 2 + 60 * 2 < x && x <= 80 + 80 * 3 + 60 * 2)
            return 23; // page up
        else if (80 + 80 * 3 + 60 * 3 < x && x <= 80 + 80 * 4 + 60 * 3)
            return 24; // page down
        else if (80 + 80 * 4 + 60 * 4 < x && x <= 80 + 80 * 5 + 60 * 4)
            return 25; // enter
    }

    return 0;
}

void *pay_loop(void *arg)
{
    pthread_detach(pthread_self());
    Bmp pay("./img/pay.bmp");
    pay.show();
    // sleep(*(int *)arg);
    for (int i = *(int *)arg; i >= 0 && ex_count == false; i--)
    {
        wb->setBox(48, 48, 0xe7e7e7e7, 0, 0);
        wb->show(to_string(i));
        sleep(1);
    }
    sem_post(&sem);
    cout << __FUNCTION__ << endl;
}

void *send_to_ser(void *arg)
{
    pthread_detach(pthread_self());

    TcpSocket sock;

    int sign = sock.connectHost("192.168.2.29", 54213);
    if (sign == 0)
    {

        sleep(5);

        string str;

        char tmp[128 + 1];
        for (int i = 0; i < 30; i++)
        {
            if (head[i].count != 0)
            {
                memset(tmp, 0, 128 + 1);
                sprintf(tmp, "{\"name\": \"%s\", \"sales\": %d, \"snack\":%d, \"al_sales\": %d},\n", head[i].name().c_str(), head[i].count, head[i].stock() - head[i].count, head[i].sales() + head[i].count);
                head[i].buy();
                str.append(tmp);
            }
        }

        cout << str;
        sock.send(str);
    }

    sem_post(&sem);

    cout << __FUNCTION__ << endl;
    ex = true;
    ex_count = true;
}

void *exit_pay(void *arg)
{
    pthread_detach(pthread_self());
    while (21 != cart_event())
        pass;
    sem_post(&sem);
    cout << __FUNCTION__ << endl;
    ex = true;

    ex_count = true;
}

void get_sem_ret()
{
    pthread_t tid;
    int count = 30;
    pthread_create(&tid, NULL, pay_loop, (void *)&count); // 等待支付
    pthread_create(&tid, NULL, exit_pay, NULL);           // 提前退出支付
    pthread_create(&tid, NULL, send_to_ser, NULL);
    sem_wait(&sem);
}

void show_cart()
{
    float price = 0;
    int k = 0;
    wb->setFont(24, 0xffffffff, 8, 8);
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            if (0 == j)
            {
                wb->setFont(20, 0x00000000, 8, 8);
                wb->setBox(400, 48 * 7, 0xffffffff, 25 + 400 * i, 48 * (j + 1));
                wb->show("商品名");

                wb->setBox(50, 48, 0xffffffff, 25 + 400 * i + 150, 48 * (j + 1));
                wb->show("单价");

                wb->setBox(50, 48, 0xffffffff, 25 + 400 * i + 150 + 50, 48 * (j + 1));
                wb->show("数量");

                wb->setBox(100, 48, 0xffffffff, 25 + 400 * i + 150 + 50 + 50, 48 * (j + 1));
                wb->show("总价");

                wb->setFont(24, 0x00000000, 8, 8);
            }
            else
            {
                for (; k < 30; k++)
                {
                    if (head[k].count != 0)
                    {
                        wb->setBox(150, 48, 0xffffffff, 25 + 400 * i, 48 * (j + 1));
                        wb->show(head[k].name());

                        wb->setBox(50, 48, 0xffffffff, 25 + 400 * i + 150, 48 * (j + 1));
                        wb->show(head[k].price());

                        wb->setBox(50, 48, 0xffffffff, 25 + 400 * i + 150 + 50, 48 * (j + 1));
                        wb->show(to_string(head[k].count));

                        wb->setBox(100, 48, 0xffffffff, 25 + 400 * i + 150 + 50 + 50, 48 * (j + 1));
                        char str[10];
                        memset(str, 0, 10);
                        sprintf(str, "%0.2f", atof(head[k].price().c_str()) * head[k].count);
                        price += atof(head[k].price().c_str()) * head[k].count;
                        wb->show(str);

                        k++;
                        break;
                    }
                }
            }
        }
    }

    wb->setBox(100, 48, 0xffffffff, 25 + 250, 48 * 7);
    wb->show("总价");

    char str[10];
    memset(str, 0, 10);
    sprintf(str, "%0.2f", price);
    wb->setBox(100, 48, 0xffffffff, 25 + 400, 48 * 7);
    wb->show(str);
}

void cart_main()
{
    sem_init(&sem, 0, 0);

    ex = false;
    ex_count = false;

    Bmp pay_main("./img/pay_main.bmp");

    int page = 0;

    bool mark = true;
    while (1)
    {
        if (mark)
        {
            pay_main.show();

            show_cart();

            mark = false;
        }

        int sign = cart_event();

        if (21 == sign)
            break;
        else if (22 == sign)
        {
            cart(-1);
            // 清理购物车
            show_cart();
        }
        else if (23 == sign)
        {
            page--;

            page %= max_page;

            mark = true;
        }
        else if (24 == sign)
        {
            page++;

            page %= max_page;

            mark = true;
        }
        else if (25 == sign)
        {
            get_sem_ret();

            if (ex == true)
                return;

            mark = true;
        }
    }
}

int main_event()
{
    int x = -1, y = -1;
    tou->wait(x, y);

    if (425 < y)
    {
        if (80 < x && x <= 80 + 80)
            return 21; // exit();
        else if (80 + 80 + 60 < x && x <= 80 + 80 * 2 + 60 * 1)
            return 22; // clear cart
        else if (80 + 80 * 2 + 60 * 2 < x && x <= 80 + 80 * 3 + 60 * 2)
            return 23; // page up
        else if (80 + 80 * 3 + 60 * 3 < x && x <= 80 + 80 * 4 + 60 * 3)
            return 24; // page down
        else if (80 + 80 * 4 + 60 * 4 < x && x <= 80 + 80 * 5 + 60 * 4)
            return 25; // enter
    }
    else if (25 + 148 < y && y < 25 + 148 + 48)
    {
        if (25 < x && x < 25 + 48)
        {
            return 100;
        }
        else if (25 + 100 < x && x < 25 + 148)
        {
            return 101;
        }
        else if (25 + 150 < x && x < 25 + 48 + 150)
        {
            return 110;
        }
        else if (25 + 100 + 150 < x && x < 25 + 148 + +150)
        {
            return 111;
        }
        else if (25 + 150 * 2 < x && x < 25 + 48 + 150 * 2)
        {
            return 120;
        }
        else if (25 + 100 + 150 * 2 < x && x < 25 + 148 + 150 * 2)
        {
            return 121;
        }
        else if (25 + 150 * 3 < x && x < 25 + 48 + 150 * 3)
        {
            return 130;
        }
        else if (25 + 100 + 150 * 3 < x && x < 25 + 148 + 150 * 3)
        {
            return 131;
        }
        else if (25 + 150 * 4 < x && x < 25 + 48 + 150 * 4)
        {
            return 140;
        }
        else if (25 + 100 + 150 * 4 < x && x < 25 + 148 + 150 * 4)
        {
            return 141;
        }
    }
    else if (25 + 148 + 200 < y && 25 + 148 + 48 + 200)
    {
        if (25 < x && x < 25 + 48)
        {
            return 150;
        }
        else if (25 + 100 < x && x < 25 + 148)
        {
            return 151;
        }
        else if (25 + 150 < x && x < 25 + 48 + 150)
        {
            return 160;
        }
        else if (25 + 100 + 150 < x && x < 25 + 148 + +150)
        {
            return 161;
        }
        else if (25 + 150 * 2 < x && x < 25 + 48 + 150 * 2)
        {
            return 170;
        }
        else if (25 + 100 + 150 * 2 < x && x < 25 + 148 + 150 * 2)
        {
            return 171;
        }
        else if (25 + 150 * 3 < x && x < 25 + 48 + 150 * 3)
        {
            return 180;
        }
        else if (25 + 100 + 150 * 3 < x && x < 25 + 148 + 150 * 3)
        {
            return 181;
        }
        else if (25 + 150 * 4 < x && x < 25 + 48 + 150 * 4)
        {
            return 190;
        }
        else if (25 + 100 + 150 * 4 < x && x < 25 + 148 + 150 * 4)
        {
            return 191;
        }
    }

    return 0;
}

void show_main_product(int page = 0)
{
    sort(head.begin(), head.end(), cmp);
    int index = 0 + page * 10;

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            Bmp bmp(head[index + i * 5 + j].path());
            bmp.show(25 + 150 * j, 25 + 200 * i);

            wb->setBox(52, 48, 0xff000000, 25 + 150 * j + 48, 25 + 200 * i + 148);
            wb->setFont(34, 0xffffffff, 0, 8);
            wb->show(head[index + i * 5 + j].price());
        }
    }
}

int main(int argc, char const *argv[])
{
    product_json_parse();
    int page = 0;
    Bmp menu("./img/main.bmp");
    bool mark = true;
    while (1)
    {
        // Bmp adv("./img/adv.bmp");
        // adv.show();
        // tou->wait(); // 待机状态显示广告

        if (mark)
        {
            menu.show();
            show_main_product(page);
            mark = false;
        }

        int sign = main_event();

        if (21 == sign)
            exit_loop();
        else if (22 == sign)
            cart(-1); // 清理购物车
        else if (23 == sign)
        {
            page--;
            page = (page < 0) ? (max_page - 1) : ((page > 2) ? 0 : page);
            // page %= max_page;
            mark = true;
        }
        else if (24 == sign)
        {
            page++;
            page = (page < 0) ? (max_page - 1) : ((page > 2) ? 0 : page);
            // page %= max_page;
            mark = true;
        }
        else if (25 == sign)
        {
            cart_main();
            mark = true;
        }
        else if (0 == sign)
            ;
        else
        {
            string s = to_string(sign);
            cart(page * 10 + (s[0] - '0' - 1) * 5 + (s[1] - '0'), s[2] == '0' ? false : true);
        }
    }
    return 0;
}
