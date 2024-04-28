#ifndef TOOL_H
#define TOOL_H
#include <ctime>
#include <cstdio>
#include<chrono>
//using namespace std;

namespace common {

static int pix_width = 1280;
static int pix_height = 720;


inline unsigned long long getLoaclTime(){
//    time_t nowtime;
//    //首先创建一个time_t 类型的变量nowtime
//    struct tm* p;
//    //然后创建一个新时间结构体指针 p
//    time(&nowtime);
//    //使用该函数就可得到当前系统时间，使用该函数需要将传入time_t类型变量nowtime的地址值。
//    p = localtime(&nowtime);
//    //由于此时变量nowtime中的系统时间值为日历时间，我们需要调用本地时间函数p=localtime（time_t* nowtime）将nowtime变量中的日历时间转化为本地时间，存入到指针为p的时间结构体中。不改的话，可以参照注意事项手动改。
//    printf("%02d:%02d:%02d\n",p->tm_hour,p->tm_min,p->tm_sec);

    auto timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
//    char bufTime[16]{ 0 };
//    sprintf(bufTime, "%lld", timeNow.count());
//    printf("time:%lld", timeNow.count());
    return timeNow.count();

}

inline int getPixWidth(){
    return pix_width;
}

inline int getPixHeight(){
    return pix_height;
}


inline void setPixWidth(const int & width){
    pix_width = width;
}

inline void setPixHeight(const int & height){
    pix_height = height;
}

}



#endif // TOOL_H
