#include "win.h"
#include "http.h"
extern void*calloc(Z,Z);extern void free(void*);extern int puts(const char*);
void*GetProcessHeap(void){return (void*)1;}void*HeapAlloc(H h,U f,Z n){return calloc(1,n);}int HeapFree(H h,U f,H p){free(p);return 1;}
int MultiByteToWideChar(U a,U b,const char*s,int n,W*out,int cap){if(out&&cap){out[0]='x';out[1]=0;}return 2;}
static U failure,httpStatus=200;static int sends,closes,readOnce,failFirst;
U GetLastError(void){return failure;}
H WinHttpOpen(const W*a,U b,const W*c,const W*d,U e){return (H)1;}
H WinHttpConnect(H a,const W*b,unsigned short c,U d){return (H)2;}
H WinHttpOpenRequest(H a,const W*b,const W*c,const W*d,const W*e,const W**f,U g){readOnce=0;return (H)3;}
int WinHttpSetTimeouts(H a,int b,int c,int d,int e){return 1;}
int WinHttpSendRequest(H a,const W*b,U c,H d,U e,U f,Z g){sends++;return !failure||(failFirst&&sends>1);}
int WinHttpReceiveResponse(H a,H b){return 1;}
int WinHttpQueryHeaders(H a,U b,const W*c,H out,U*d,U*e){*(U*)out=httpStatus;return 1;}
int WinHttpReadData(H a,H out,U n,U*got){if(readOnce++){*got=0;return 1;}memcpy(out,"ok",2);*got=2;return 1;}
int WinHttpCloseHandle(H a){closes++;return 1;}
#define ASSERT(x) if(!(x)){puts("FAIL " #x);return 1;}
int main(void){
 U st=0;char*d=requestData("example.com","/",0,0,&st);ASSERT(st==200&&sends==1&&!strcmp(d,"ok")&&closes==3);drop(d);
 sends=closes=0;failure=12002;failFirst=1;d=requestData("example.com","/",0,0,&st);ASSERT(st==200&&sends==2&&closes==6);drop(d);
 sends=closes=0;failure=12175;failFirst=0;d=requestData("example.com","/",0,0,&st);ASSERT(st==112175&&sends==1&&!*d&&closes==3);drop(d);
 sends=closes=0;failure=0;httpStatus=403;d=requestData("example.com","/",0,0,&st);ASSERT(st==403&&sends==1&&closes==3);drop(d);
 sends=0;failure=12007;d=requestData("example.com","/",0,0,&st);ASSERT(st==112007&&sends==1);drop(d);
 sends=0;failure=12002;d=requestData("example.com","/",0,0,&st);ASSERT(st==112002&&sends==2);drop(d);
 puts("PASS HTTP: native error preservation, bounded transient retry, no retry on 403/TLS/DNS, handle cleanup.");return 0;
}
