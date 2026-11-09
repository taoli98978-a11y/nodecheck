#define NODECHECK_TEST
#include "win.h"
#include "geo.h"
#include "lookup.h"
extern void*calloc(Z,Z);extern void free(void*);extern int puts(const char*),printf(const char*,...);extern int strcasecmp(const char*,const char*);
void*GetProcessHeap(void){return (void*)1;}void*HeapAlloc(void*h,U f,Z n){return calloc(1,n);}int HeapFree(void*h,U f,void*p){free(p);return 1;}
int _stricmp(const char*a,const char*b){return strcasecmp(a,b);}
#include <stdarg.h>
extern int vsnprintf(char*,Z,const char*,va_list);
int _snprintf(char*out,Z n,const char*fmt,...){va_list ap;va_start(ap,fmt);int r=vsnprintf(out,n,fmt,ap);va_end(ap);return r;}
extern void*popen(const char*,const char*);extern int pclose(void*);extern char*fgets(char*,int,void*);extern Z fread(void*,Z,Z,void*);
static char*requestData(const char*host,const char*path,const char*body,const char*headers,U*status){char url[4096],hex[8192],cmd[8300];fmt(url,"https://%s%s",host,path);const char*digits="0123456789abcdef";int i;for(i=0;url[i]&&i<4095;i++){hex[i*2]=digits[(unsigned char)url[i]>>4];hex[i*2+1]=digits[url[i]&15];}hex[i*2]=0;fmt(cmd,"python3 test_fetch.py %s",hex);void*f=popen(cmd,"r");char*data=alloc(2097153);U code=0;if(f){char h[30];if(fgets(h,sizeof(h),f))code=atoi(h);fread(data,1,2097152,f);pclose(f);}if(status)*status=code;return data;}
static void progress(void*ctx){Lookup*l=ctx;printf("PROGRESS home=%s area=%s\n",l->home,l->area);}
int main(int argc,char**argv){if(argc!=5&&argc!=6){puts("usage: core_cli STATE CITY LAT LON");return 2;}Geo g={0};cp(g.country,"US");cp(g.state,argv[1]);cp(g.city,argv[2]);cp(g.lat,argv[3]);cp(g.lon,argv[4]);Candidate c[10];int n=postalCandidates(&g,c,10);for(int i=0;i<n;i++)printf("CANDIDATE %s %.1f\n",postalData[c[i].index].zip,c[i].km);Lookup*l=alloc(sizeof(Lookup));volatile int generation=1;if(argc==6)seedLocal(&g,l);else lookupRegion(&g,l,&generation,1,progress,l);puts("RESULT");puts(l->home);puts(l->area);puts(l->copy);puts(l->details);int ok=*l->home&&*l->area;drop(l);return ok?0:1;}
