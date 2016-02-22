#ifndef _HOOK_H_
#define _HOOK_H_


namespace hook {

enum {
	HOOK_SUCCESS = 0,
	HOOK_FAILURE = 1
};

class Penetrator
{
  public:
	Penetrator()
      : module_path_(nullptr)
    {}

  	bool ResolveAnalysisModule();

  private:
  	char* module_path_;
};

}


#endif