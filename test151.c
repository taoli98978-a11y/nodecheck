#define main priorTests
#include "test150.c"
#undef main
int main(void){
 CHECKIT(priorTests()==0);
 Geo a={0};cp(a.country,"US");cp(a.state,"Texas");cp(a.city,"Fort Worth");cp(a.zip,"76102");cp(a.lat,"32.7254134");cp(a.lon,"-97.3208492");Lookup l={0};seedLocal(&a,&l);
 CHECKIT(hasCode(l.area,"817")&&hasCode(l.area,"682"));CHECKIT(strstr(l.copy,"沃思堡")&&strstr(l.copy,"  电话区号：")&&!strchr(l.copy,'\n'));
 char raw[1600],out[6800];regionCopy(&a,"",l.area,1,raw,sizeof(raw));CHECKIT(strstr(raw,"邮编：未提供")&&strstr(raw,"817")&&strstr(raw,"未核查"));
 const char*node="socks5://example.test:10001:user:p+:x";combinedCopy(out,sizeof(out),node,l.copy,1);CHECKIT(!strncmp(out,node,strlen(node)));CHECKIT(out[strlen(node)]==' '&&out[strlen(node)+1]==' '&&out[strlen(node)+2]!=' ');CHECKIT(!strcmp(out+strlen(node)+2,l.copy));
 combinedCopy(out,sizeof(out),node,l.copy,0);CHECKIT(!strcmp(out,node));
 Job j={0};j.found[0]=j.found[1]=1;j.geo[0]=j.geo[1]=a;j.result[0]=l;cp(j.geo[1].zip,"76104");CHECKIT(resultFor(&j,1)==&j.result[0]);regionCopy(&j.geo[1],j.geo[1].zip,resultFor(&j,1)->area,1,raw,sizeof(raw));CHECKIT(strstr(raw,"76104")&&strstr(raw,"817"));
 cp(j.geo[1].city,"Austin");cp(j.result[1].area,"512、737");CHECKIT(resultFor(&j,1)==&j.result[1]);regionCopy(&j.geo[1],j.geo[1].zip,resultFor(&j,1)->area,1,raw,sizeof(raw));CHECKIT(strstr(raw,"512")&&!strstr(raw,"817"));
 a.city[0]=a.zip[0]=0;Lookup st={0};seedLocal(&a,&st);CHECKIT(*st.area&&!*st.home&&strstr(st.copy,"州级参考"));
 regionCopy(&j.geo[1],"76104","",1,raw,sizeof(raw));CHECKIT(strstr(raw,"电话区号：未取得"));
 puts("PASS 1.5.1: two ASCII space separators, complete node preservation, all regional copy variants include area codes, same-city sharing, different-city isolation, missing-ZIP independence, state-only labeling.");return 0;
}
