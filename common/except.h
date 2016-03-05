#ifndef _EXCEPT_H_
#define _EXCEPT_H_


class BadProbe : public std::exception
{
  public:
    const char* what() const throw ()
    {
        return "Fail to interact with the probed process.";
    }
};


#endif