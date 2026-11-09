#define NODECHECK_TEST
#include "main.c"
static const char*mockResponse;static U mockStatus;static int requestCount;
static char*requestData(const char*h,const char*p,const char*b,const char*heads,U*status){requestCount++;if(status)*status=mockStatus;char*d=alloc(mockResponse?strlen(mockResponse)+1:1);if(mockResponse)copyto(d,strlen(mockResponse)+1,mockResponse);return d;}
extern void* calloc(Z,Z);extern void free(void*);extern int puts(const char*);
void* GetProcessHeap(void){return (void*)1;} void* HeapAlloc(void*h,U flag,Z n){return calloc(1,n);}int HeapFree(void*h,U flag,void*p){free(p);return 1;}
extern int strcasecmp(const char*,const char*);int _stricmp(const char*a,const char*b){return strcasecmp(a,b);}
#include <stdarg.h>
extern int vsnprintf(char*,Z,const char*,va_list);
int _snprintf(char*out,Z size,const char*format,...){va_list ap;va_start(ap,format);int r=vsnprintf(out,size,format,ap);va_end(ap);return r;}
static unsigned char out[4096],incoming[4096];static int used,rd,total;
int send(Sock s,const char*b,int n,int f){if(n>5)n=5;memcpy(out+used,b,n);used+=n;return n;}
int recv(Sock s,char*b,int n,int f){if(n>2)n=2;if(n>total-rd)n=total-rd;memcpy(b,incoming+rd,n);rd+=n;return n;}
int EncryptMessage(Sec*s,U x,Desc*d,U a){return -1;}int DecryptMessage(Sec*s,Desc*d,U a,U*q){return -1;}
#define CHECKIT(x) if(!(x)){puts("FAILED " #x);return 1;}
int protocolTests(void){
 Node n={0};n.scheme=2;char input[]="us.example:10001:user-zip-02108:p+:x";CHECKIT(parseNode(input,&n));CHECKIT(!strcmp(n.pass,"p+:x")&&!strcmp(n.port,"10001")&&n.scheme==2);
 char v6[]="https://[2001:db8::1]:123:u:p";CHECKIT(parseNode(v6,&n)&&n.scheme==1&&!strcmp(n.host,"2001:db8::1"));
 char bad[]="broken:abc";CHECKIT(!parseNode(bad,&n));
 unsigned char reply[]={5,2,1,0,5,0,0,1,127,0,0,1,0,20};memcpy(incoming,reply,sizeof(reply));total=sizeof(reply);Stream*s=calloc(1,sizeof(Stream));
 CHECKIT(socks(s,"sample.test","u","p+"));unsigned char expect[]={5,1,2,1,1,'u',2,'p','+',5,1,0,3,11,'s','a','m','p','l','e','.','t','e','s','t',1,187};
 extern int memcmp(const void*,const void*,Z);CHECKIT(used==sizeof(expect)&&!memcmp(out,expect,sizeof(expect)));
 used=rd=0;incoming[1]=255;CHECKIT(!socks(s,"sample.test","u","p+"));
 used=rd=0;const char*resp="HTTP/1.1 200 Connection established\r\n\r\n";total=strlen(resp);memcpy(incoming,resp,total);CHECKIT(httpTunnel(s,"sample.test","u","p+"));out[used]=0;CHECKIT(strstr((char*)out,"Basic dTpwKw==\r\n")!=0);
 char json[]="{\"ip\":\"1.2.3.4\",\"address\":{\"city\":\"Boston\",\"postcode\":\"02108\"}}";Json j=doc(json);char zip[32],city[50];int a=field(&j,0,"address");fld(&j,a,"postcode",zip);fld(&j,a,"city",city);char place[90];fmt(place,"%s, Massachusetts %s",city,zip);CHECKIT(!strcmp(place,"Boston, Massachusetts 02108"));drop(j.t);free(s);
 puts("PASS: input parsing, IPv6 proxy syntax, special passwords, SOCKS5 authentication/connect and rejection, HTTP CONNECT basic authentication, fragmented streams, nested JSON and leading-zero postal copy.");return 0;}
