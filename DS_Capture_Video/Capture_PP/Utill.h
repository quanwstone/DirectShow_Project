#pragma once

#define DEF_BUF_MAX 256

#define DEF_SAFE_DELETE_BUF(p)\
{\
	if(p)\
	{\
		delete[] p;\
		p = nullptr;\
	}\
}

#define DEF_SAFE_DELETE_OBJECT(p)\
{\
	if(p)\
	{\
		delete p;\
		p = nullptr;\
	}\
}
#define DEF_SAFE_DELETE_COM(p)\
{\
	if(p)\
	{\
		p->Release();\
		p = nullptr;\
	}\
}