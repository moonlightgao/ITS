/*
 * @file      Application.h
 * @brief     Application header file
 * @author    Longsheng Du
 * @version   v0.1
 * @date      2019/8/16
 * @copyright 2019, Deepglint All rights reserved.
 */

#ifndef _FRAMEWORK_APPLICATION_H_
#define _FRAMEWORK_APPLICATION_H_

#include <memory>
#include <iostream>

namespace utility {

class Application
{
private:
    static std::unique_ptr<Application> &CurrApp()
    {
        static std::unique_ptr<Application> app_ptr;
        return app_ptr;
    }

    virtual void run()  = 0;
public:
    virtual ~Application() = default;

    template<class App>
    static void Init()
    {
        CurrApp().reset(new App());
    }

    static void Run()
    {
        if(CurrApp() == nullptr)
        {
            std::cerr << "Application not initialized!\n";
        }
        else
        {
            // Application run 
            CurrApp()->run();
            CurrApp().reset();
        }
    }
};

} // namespace utility

#endif /* _FRAMEWORK_APPLICATION_H_ */
