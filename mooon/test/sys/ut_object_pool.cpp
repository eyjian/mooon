#include <mooon/sys/object_pool.h>
#include <stdio.h>
#include <vector>

class X: public mooon::sys::CPoolObject
{
public:
    X()
    {
        reset();
    }

    void reset()
    {
        _m = -1;
    }

    void s(int m)
    {
        _m = m;
    }

    void p()
    {
        printf("%d\n", _m);
    }

private:
    int _m;
};

int main()
{
    X* x;
    uint32_t i;
    uint32_t pool_size = 10;
    mooon::sys::CRawObjectPool<X> pool(false);
    pool.create(pool_size);

    std::vector<X*> vec_x;
    for (i=0; i<=pool_size; ++i)
    {
        printf("[%d] ", i);

        x = pool.borrow();
        if (NULL == x)
        {
            printf("borrow nothing\n");
        }
        else
        {
            x->s(2015);
            x->p();

            vec_x.push_back(x);
        }
    }

    for (std::vector<X*>::size_type j=0; j<vec_x.size(); ++j)
    {
        x = vec_x[j];
        pool.pay_back(x);
    }

    x = pool.borrow();
    x->s(501);
    x->p();
    pool.pay_back(x);

    return 0;
}
