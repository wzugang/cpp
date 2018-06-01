//回调函数同时支持成员函数和静态函数

#include <iostream>
#include <functional>
#include <list>

using namespace std;

// 通知结构体，包含通知的消息
class timeup_notify_obj
{
public:
    int cur_time;
};

// 函数对象基类，包含纯虚的operator()接口待子类实现。
class timeup_notify_func : public unary_function<timeup_notify_obj, int>
{
public:
    virtual result_type operator()(const argument_type& notify_arg) const = 0;
};

// 成员函数对象，其中保存有注册时的对象指针，以便调用。
template<typename classname>
class timeup_notify_memfunc : public timeup_notify_func
{
public:
    timeup_notify_memfunc(classname& in_callobj, result_type (classname::*in_func)(argument_type))
        : func(bind1st(mem_fun(in_func), &in_callobj))
    {
    }
    virtual result_type operator()(const argument_type& notify_arg) const
    {
        return func(notify_arg);
    }
private:
    binder1st<mem_fun1_t<result_type, classname, argument_type> > func;
};

// const成员函数对象，和成员函数对象唯一的区别是其this指针为const。
template<typename classname>
class timeup_notify_memfunc_const : public timeup_notify_func
{
public:
    timeup_notify_memfunc_const(const classname& in_callobj, result_type (classname::*in_func)(argument_type) const)
        : func(bind1st(mem_fun(in_func), &in_callobj))
    {
    }
    virtual result_type operator()(const argument_type& notify_arg) const
    {
        return func(notify_arg);
    }
private:
    binder1st<const_mem_fun1_t<result_type, classname, argument_type> > func;
};

// 全局/静态函数对象。
class timeup_notify_static_func : public timeup_notify_func
{
public:
    timeup_notify_static_func(result_type (*in_func)(argument_type)) : func(ptr_fun(in_func))
    {
    }
    virtual result_type operator()(const argument_type& notify_arg) const
    {
        return func(notify_arg);
    }
private:
    pointer_to_unary_function<argument_type, result_type> func;
};

// 下面是一些辅助函数，用来生成对应类型的函数对象。
template<typename classname>
inline timeup_notify_memfunc<classname>* creat_timeup_func(classname& in_callobj, int (classname::*in_func)(timeup_notify_obj))
{
    return new timeup_notify_memfunc<classname>(in_callobj, in_func);
}

template<typename classname>
inline timeup_notify_memfunc_const<classname>* creat_timeup_func(const classname& in_callobj, int (classname::*in_func)(timeup_notify_obj) const)
{
    return new timeup_notify_memfunc_const<classname>(in_callobj, in_func);
}

inline timeup_notify_static_func* creat_timeup_func(int (*in_func)(timeup_notify_obj))
{
    return new timeup_notify_static_func(in_func);
}

class BaseA
{
public:
    virtual void func_BaseA(){}
};

class BaseB
{
public:
    virtual void func_BaseB(){}
};

// 这里故意用一个多重继承，来体现其相对于void*指针会更安全。
class A : public BaseA, public BaseB
{
public:
    virtual int update(timeup_notify_obj timeup_obj)
    {
        return timeup_obj.cur_time + my_time;
    }

    int update_const(timeup_notify_obj timeup_obj) const
    {
        return timeup_obj.cur_time * my_time;
    }

    static int update_static(timeup_notify_obj timeup_obj)
    {
        return timeup_obj.cur_time;
    }

    int my_time;
};

int update_global(timeup_notify_obj timeup_obj)
{
    return timeup_obj.cur_time + 100;
}

int main(int argc, char* argv[])
{
    list<timeup_notify_func*> notify_func_list;
    A a;
    a.my_time = 2;
    notify_func_list.push_back(creat_timeup_func(a, &A::update));
    A b;
    b.my_time = 5;
    notify_func_list.push_back(creat_timeup_func(b, &A::update_const));
    notify_func_list.push_back(creat_timeup_func(A::update_static));
    notify_func_list.push_back(creat_timeup_func(update_global));

    timeup_notify_obj timeup;
    timeup.cur_time = 4;

    // 循环遍历调用所有类型的函数
    for (list<timeup_notify_func*>::iterator it = notify_func_list.begin(); it != notify_func_list.end(); ++it)
    {
        int ret = (*(*it))(timeup);
        cout << ret << endl;
    }

    return 0;
}

