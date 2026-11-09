#include "http.h"
typedef struct {int a,b,end;} Token;
typedef struct {char*s;Token*t;int n,pos,cap;} Json;
static void skip(Json*j){while(j->s[j->pos]&&(unsigned char)j->s[j->pos]<=32)j->pos++;}
static int parse(Json*j,int depth){
 skip(j);if(depth>48||j->n>=j->cap||!j->s[j->pos])return 0;int at=j->n++;Token*t=j->t+at;char ch=j->s[j->pos];t->a=j->pos;
 if(ch=='{'||ch=='['){j->pos++;skip(j);char close=ch=='{'?'}':']';while(j->s[j->pos]!=close){if(!parse(j,depth+1))return 0;skip(j);if(j->s[j->pos]==','||j->s[j->pos]==':')j->pos++;else if(j->s[j->pos]!=close)return 0;skip(j);}j->pos++;}
 else if(ch=='"'){j->pos++;while(j->s[j->pos]&&j->s[j->pos]!='"'){if(j->s[j->pos]=='\\'&&j->s[j->pos+1])j->pos++;j->pos++;}if(!j->s[j->pos])return 0;j->pos++;}
 else{while(j->s[j->pos]&&!strchr(",]} \t\r\n",j->s[j->pos]))j->pos++;if(t->a==j->pos)return 0;}
 t->end=j->n;t->b=j->pos;return 1;
}
static Json doc(char*s){Json j={s,alloc(16384*sizeof(Token)),0,0,16384};if(!parse(&j,0))j.n=0;return j;}
static int field(Json*j,int o,const char*key){if(o<0||o>=j->n||j->s[j->t[o].a]!='{')return -1;for(int i=o+1;i+1<j->t[o].end;){Token*t=j->t+i;if(t->b-t->a==(int)strlen(key)+2&&!strncmp(j->s+t->a+1,key,strlen(key)))return i+1;i=j->t[i+1].end;}return -1;}
static void getfield(Json*j,int o,const char*k,char*dst,int cap){int i=field(j,o,k);*dst=0;if(i<0)return;int a=j->t[i].a,b=j->t[i].b;if(j->s[a]=='"'){a++;b--;}else if(!strncmp(j->s+a,"null",4))return;int count=b-a;if(count>=cap)count=cap-1;memcpy(dst,j->s+a,count);dst[count]=0;}
#define fld(j,o,k,b) getfield(j,o,k,b,sizeof(b))
static void release(Json*j){drop(j->s);drop(j->t);}
static void encode(char*out,int cap,const char*in){const char*hex="0123456789ABCDEF";int k=0;for(;*in&&k<cap-4;in++){int x=(unsigned char)*in;if((x>='A'&&x<='Z')||(x>='a'&&x<='z')||(x>='0'&&x<='9')||strchr("-_.~",x))out[k++]=x;else{out[k++]='%';out[k++]=hex[x>>4];out[k++]=hex[x&15];}}out[k]=0;}
static int zip5(const char*z){if(strlen(z)!=5)return 0;for(int i=0;i<5;i++)if(z[i]<'0'||z[i]>'9')return 0;return 1;}
static int coord(const char*s,int max){if(!*s||strlen(s)>18)return 0;int digits=0,dot=0;for(int i=(*s=='-');s[i];i++){if(s[i]>='0'&&s[i]<='9')digits++;else if(s[i]=='.'&&!dot)dot++;else return 0;}double x=atof(s);return digits&&x>=-max&&x<=max;}
typedef struct {char ip[64],city[160],state[160],country[16],lat[32],lon[32],zip[32];U status;} Geo;
static int geolook(Geo*g){char path[150],returned[64],ok[12];fmt(path,"/%s",g->ip);Json j=doc(requestData("ipwho.is",path,0,0,&g->status));fld(&j,0,"ip",returned);fld(&j,0,"success",ok);int valid=same(ok,"true")&&same(returned,g->ip);
 if(valid){fld(&j,0,"city",g->city);fld(&j,0,"region",g->state);fld(&j,0,"country_code",g->country);fld(&j,0,"latitude",g->lat);fld(&j,0,"longitude",g->lon);fld(&j,0,"postal",g->zip);}release(&j);return valid;}
static const char*val(const char*s){return *s?s:"未提供";}
static int ip2geo(Geo*g){char path[150],ret[64];fmt(path,"/?ip=%s&format=json",g->ip);Json j=doc(requestData("api.ip2location.io",path,0,0,&g->status));fld(&j,0,"ip",ret);
 int ok=same(ret,g->ip);if(ok){fld(&j,0,"city_name",g->city);fld(&j,0,"region_name",g->state);fld(&j,0,"country_code",g->country);fld(&j,0,"latitude",g->lat);fld(&j,0,"longitude",g->lon);fld(&j,0,"zip_code",g->zip);}release(&j);return ok&&*g->state&&*g->country;}

#include "places.h"
