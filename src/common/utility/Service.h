/*
 * @file      Service.h
 * @brief     Service header file
 * @author    Longsheng Du
 * @version   v0.1
 * @date      2019/8/16
 * @copyright 2019, Deepglint All rights reserved.
 */

#ifndef _FRAMEWORK_SERVICE_H_
#define _FRAMEWORK_SERVICE_H_

#include <vector>

namespace utility {

class Service
{
private:
    static std::vector<Service*> &Register()
    {
        static std::vector<Service*> reg;
        return reg;
    }

    virtual void start()  = 0;
    virtual void stop()   = 0;
protected:
    virtual ~Service() = default;
public:
    template<class Obj>
    static void Init()
    {
        Register().push_back( &Obj::service() );
    }

    static void Start()
    {
        for(auto sp=Register().begin(); sp!=Register().end(); sp++)
        {
            (*sp) -> start();
        }
    }

    static void Stop()
    {
        for(auto sp=Register().rbegin(); sp!=Register().rend(); sp++)
        {
            (*sp) -> stop();
        }
    }
};

} // namespace utility

#endif /* _FRAMEWORK_SERVICE_H_ */
