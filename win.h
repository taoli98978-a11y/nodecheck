#include <stdint.h>
#include <stddef.h>
typedef void *H; typedef unsigned int U; typedef uint16_t W; typedef uintptr_t Z; typedef intptr_t I;
#define ws(x) ((const W*)u##x)
extern H GetProcessHeap(void),HeapAlloc(H,U,Z); extern int HeapFree(H,U,H);
static void *alloc(Z n){return HeapAlloc(GetProcessHeap(),8,n);}
static void drop(void *p){if(p)HeapFree(GetProcessHeap(),0,p);}
extern int MultiByteToWideChar(U,U,const char*,int,W*,int),WideCharToMultiByte(U,U,const W*,int,char*,int,const char*,int*);
extern void ExitProcess(U); extern H GetModuleHandleW(const W*),CreateThread(H,Z,U(*)(H),H,U,U*);extern int CloseHandle(H);
extern int lstrlenW(const W*); extern void *memcpy(void*,const void*,Z),*memset(void*,int,Z);
extern Z strlen(const char*);extern int strcmp(const char*,const char*),strncmp(const char*,const char*,Z),_stricmp(const char*,const char*),atoi(const char*);
extern char *strstr(const char*,const char*),*strchr(const char*,int),*strrchr(const char*,int);extern double atof(const char*);
extern int _snprintf(char*,Z,const char*,...);
#define fmt(b,...) do{_snprintf(b,sizeof(b)-1,__VA_ARGS__);(b)[sizeof(b)-1]=0;}while(0)
static W *wide(const char*s){int n=MultiByteToWideChar(65001,0,s,-1,0,0);W*w=alloc((n+1)*2);MultiByteToWideChar(65001,0,s,-1,w,n);return w;}
static int same(const char*a,const char*b){return *a&&*b&&!_stricmp(a,b);}
static void append(char*buf,Z cap,const char*add){Z a=strlen(buf),n=strlen(add);if(a+1>=cap)return;if(n>=cap-a)n=cap-a-1;memcpy(buf+a,add,n);buf[a+n]=0;}
static void copyto(char*b,Z cap,const char*s){*b=0;append(b,cap,s);}
#define cp(b,s) copyto(b,sizeof(b),s)
#define add(b,s) append(b,sizeof(b),s)
