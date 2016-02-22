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
      : module_path_(nullptr),
      	dex_path_(nullptr)
    {}

  	bool CraftAnalysisModulePath();
  	bool CraftDexPrivatePath();

  private:
  	char* module_path_;
  	std::unique_ptr<char> dex_path_;
};

}

#endif