#include "win.h"
#include "net.h"
#include "geo.h"
#include "lookup.h"
typedef struct{int x,y,r,b;} Rect;
typedef struct{U style;I(*proc)(H,U,Z,I);int ce,we;H inst,icon,cur,brush;const W*menu,*name;} Class;
typedef struct{H hwnd;U msg;Z wp;I lp;U time;int x,y;U extra;} Msg;
extern H LoadIconW(H,const W*);extern int MessageBoxW(H,const W*,const W*,U);
extern unsigned short RegisterClassW(Class*);extern H CreateWindowExW(U,const W*,const W*,U,int,int,int,int,H,H,H,H),LoadCursorW(H,const W*),GetStockObject(int),CreateFontW(int,int,int,int,int,U,U,U,U,U,U,U,U,const W*);
extern int SetProcessDPIAware(void),ShowWindow(H,int),UpdateWindow(H),GetMessageW(Msg*,H,U,U),TranslateMessage(Msg*),IsDialogMessageW(H,Msg*),GetClientRect(H,Rect*),MoveWindow(H,int,int,int,int,int),SetWindowTextW(H,const W*),GetWindowTextW(H,W*,int),EnableWindow(H,int),PostMessageW(H,U,Z,I),SetWindowPos(H,H,int,int,int,int,U),DestroyWindow(H),OpenClipboard(H),CloseClipboard(void),EmptyClipboard(void);
extern H SetFocus(H),SetClipboardData(U,H),GlobalAlloc(U,Z),GlobalLock(H),GlobalFree(H);extern int GlobalUnlock(H);extern I DispatchMessageW(Msg*),DefWindowProcW(H,U,Z,I),SendMessageW(H,U,Z,I);extern void PostQuitMessage(int);
extern U SetTextColor(H,U),SetBkColor(H,U);extern H GetSysColorBrush(int);

enum{PASTE=1,PROTOCOL,V4,PIN,HOST,PORT,USER,PASS,SHOW,CHECK,NODE,RAW,LOC,AUDIT,TITLE,LP,LPRO,LHOST,LUSER,LPASS,FOOT,ABOUT,LOC2,DETAIL,ZIPRAW,ZIPRAW2};
static int importedUrl;static H controls[32],mainwin,inst,font;static int changing,busy,showDetails,twoGroups;static volatile int generation,activeJobs;
static char clipNode[13000],clipPlace[2][1400],summary[4000],detailText[30000],clipRaw[2][1600],clipCombined[15000];static const char*schemes[]={"http","https","socks5","socks5h"};
static H ctl(const W*cls,const W*text,U style,int id){H h=CreateWindowExW(cls[0]=='E'?0x200:0,cls,text,0x50000000|style,0,0,10,10,mainwin,(H)(Z)id,inst,0);controls[id]=h;SendMessageW(h,0x30,(Z)font,1);return h;}
static void put(int id,const char*s){W*w=wide(s);SetWindowTextW(controls[id],w);drop(w);}
static void readbox(int id,char*out,int cap){W*w=alloc(cap*2);GetWindowTextW(controls[id],w,cap);if(!WideCharToMultiByte(65001,0,w,-1,out,cap,0,0))*out=0;drop(w);}
static void mv(int i,int x,int y,int w,int h){MoveWindow(controls[i],x,y,w>1?w:1,h>1?h:1,1);}
static void layout(){Rect r;GetClientRect(mainwin,&r);int w=r.r,h=r.b;mv(TITLE,14,9,w-115,22);mv(ABOUT,w-81,7,67,25);int ids[]={LP,LPRO,LHOST,LUSER,LPASS};for(int k=0;k<5;k++)mv(ids[k],14,45+k*30,72,23);
 mv(PASTE,89,42,w-103,23);mv(PROTOCOL,89,72,94,140);mv(V4,192,73,84,23);mv(PIN,280,73,94,23);mv(HOST,89,102,w-176,23);mv(PORT,w-79,102,65,23);mv(USER,89,132,w-103,23);mv(PASS,89,162,w-174,23);mv(SHOW,w-78,162,64,23);
 int half=(w-40)/2;mv(CHECK,14,197,half,30);mv(NODE,26+half,197,w-40-half,30);mv(RAW,14,237,w-28,95);
 mv(AUDIT,14,342,w-28,h-446);int available=w-130;mv(LOC,14,h-96,twoGroups?(available-8)/2:available,28);mv(LOC2,22+(available-8)/2,h-96,(available-8)/2,28);mv(DETAIL,w-106,h-96,92,28);ShowWindow(controls[LOC2],twoGroups?5:0);mv(ZIPRAW,14,h-62,(w-36)/2,28);mv(ZIPRAW2,22+(w-36)/2,h-62,(w-36)/2,28);mv(FOOT,14,h-27,w-28,22);}
static int checked(int id){return SendMessageW(controls[id],0xf0,0,0)==1;}
static void lockform(int lock){int ids[]={PASTE,PROTOCOL,HOST,PORT,USER,PASS,V4};for(int k=0;k<7;k++)EnableWindow(controls[ids[k]],!lock);}
static void clearResult(){generation++;clipCombined[0]=clipNode[0]=clipPlace[0][0]=clipPlace[1][0]=summary[0]=detailText[0]=clipRaw[0][0]=clipRaw[1][0]=0;EnableWindow(controls[ZIPRAW],0);EnableWindow(controls[ZIPRAW2],0);put(ZIPRAW,"复制接口邮编①");put(ZIPRAW2,"复制接口邮编②");twoGroups=showDetails=0;EnableWindow(controls[NODE],0);EnableWindow(controls[LOC],0);EnableWindow(controls[LOC2],0);put(NODE,"复制节点（含密码）");put(LOC,"复制城市邮编区号");put(LOC2,"复制第二组");put(DETAIL,"详细信息");put(RAW,"等待检测。");put(AUDIT,"住宅参考邮编和当地电话区号将在这里显示。");layout();}
static int clipboard(const char*s){if(!*s)return 0;W*w=wide(s);int bytes=(lstrlenW(w)+1)*2;H m=GlobalAlloc(2,bytes);void*ptr=m?GlobalLock(m):0;int ok=0;if(ptr){memcpy(ptr,w,bytes);GlobalUnlock(m);if(OpenClipboard(mainwin)){if(EmptyClipboard()&&SetClipboardData(13,m))ok=1;CloseClipboard();}}if(m&&!ok)GlobalFree(m);drop(w);return ok;}
typedef struct{Node n;Geo geo[2];Lookup result[2];int found[2],gen,connected,geoReady;char error[500];} Job;
static const Lookup*resultFor(const Job*j,int i){return j->found[0]&&j->found[1]&&samePlace(&j->geo[0],&j->geo[1])?&j->result[0]:&j->result[i];}
enum{UPDATE=0x8010,FINISH};
static void deliver(Job*j,int msg){Job*v=alloc(sizeof(Job));*v=*j;if(!PostMessageW(mainwin,msg,0,(I)v))drop(v);}
static void progress(void*ctx){deliver(ctx,UPDATE);}
static U work(H arg){Job*j=arg;const char*e=probe(&j->n,j->geo[0].ip,j->n.v4?"api.ipify.org":"api64.ipify.org");
 if(e&&!cancelled(&generation,j->gen)&&(same(e,"出口查询未返回有效 IP。")||same(e,"代理已连接，出口查询的 TLS 验证失败。")))e=probe(&j->n,j->geo[0].ip,j->n.v4?"ipv4.icanhazip.com":"icanhazip.com");
 if(e){cp(j->error,e);goto done;}j->connected=1;deliver(j,UPDATE);if(cancelled(&generation,j->gen))goto done;
 cp(j->geo[1].ip,j->geo[0].ip);j->found[0]=geolook(&j->geo[0])&&*j->geo[0].country&&*j->geo[0].state;if(cancelled(&generation,j->gen))goto done;j->found[1]=ip2geo(&j->geo[1])&&*j->geo[1].country&&*j->geo[1].state;j->geoReady=1;if(j->found[0])seedLocal(&j->geo[0],&j->result[0]);if(j->found[1]&&(!j->found[0]||!samePlace(&j->geo[0],&j->geo[1])))seedLocal(&j->geo[1],&j->result[1]);deliver(j,UPDATE);
 if(j->found[0]&&!cancelled(&generation,j->gen))lookupRegion(&j->geo[0],&j->result[0],&generation,j->gen,progress,j);
 if(j->found[1]&&!cancelled(&generation,j->gen)&&(!j->found[0]||!samePlace(&j->geo[0],&j->geo[1])))lookupRegion(&j->geo[1],&j->result[1],&generation,j->gen,progress,j);
 done:deliver(j,FINISH);drop(j);__atomic_sub_fetch(&activeJobs,1,__ATOMIC_SEQ_CST);return 0;}
static int decodeCredential(char*s){char*d=s;while(*s){int c=(unsigned char)*s++;if(c=='%'){if(!s[0]||!s[1])return 0;int a=hexDigit(s[0]),b=hexDigit(s[1]);if(a<0||b<0)return 0;c=a*16+b;s+=2;}if(c<32||c==127)return 0;*d++=c;}*d=0;return 1;}
static int parseNode(char*s,Node*n){
 while(*s==' '||*s=='\t'||*s=='\r'||*s=='\n')s++;int len=strlen(s);while(len&&(s[len-1]==' '||s[len-1]=='\t'||s[len-1]=='\r'||s[len-1]=='\n'))s[--len]=0;
 char*p=strstr(s,"://");int prefixed=p!=0;n->urlstyle=0;if(p){*p=0;int k=0;for(;k<4&&!same(s,schemes[k]);k++);if(k==4)return 0;n->scheme=k;s=p+3;}
 char*h=s,*po,*u="",*pw="",*end,*at=prefixed?strrchr(s,'@'):0;
 /* Recognize the legacy host:port:user:password form before URI userinfo. */
 char*first=*s=='['?strchr(s,']'):strchr(s,':');if(first&&*s=='[')first=first[1]==':'?first+1:0;if(first){char*t=first+1;while(*t>='0'&&*t<='9')t++;if(t>first+1&&*t==':'&&strchr(t+1,':'))at=0;}
 if(at){n->urlstyle=1;*at=0;u=s;end=strchr(u,':');if(!end)return 0;*end=0;pw=end+1;if(!decodeCredential(u)||!decodeCredential(pw))return 0;h=at+1;}
 if(*h=='['){h++;end=strchr(h,']');if(!end||end[1]!=':')return 0;*end=0;po=end+2;}else{end=strchr(h,':');if(!end)return 0;*end=0;po=end+1;}
 if(!at){end=strchr(po,':');if(end){*end=0;u=end+1;end=strchr(u,':');if(!end)return 0;*end=0;pw=end+1;}else if(prefixed)n->urlstyle=1;else return 0;}
 if(!*h||!*po||strlen(h)>=sizeof(n->host)||strlen(po)>=sizeof(n->port)||strlen(u)>=sizeof(n->user)||strlen(pw)>=sizeof(n->pass))return 0;
 for(char*t=h;*t;t++)if((unsigned char)*t<=32||*t=='/'||*t=='?'||*t=='#'||*t=='@')return 0;
 for(char*t=po;*t;t++)if(*t<'0'||*t>'9')return 0;if(atoi(po)<1||atoi(po)>65535)return 0;
 cp(n->host,h);cp(n->port,po);cp(n->user,u);cp(n->pass,pw);return 1;
}
static void encodeCredential(const char*s,char*d){const char*hex="0123456789ABCDEF";while(*s){unsigned char c=*s++;if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~'||c=='+')*d++=c;else{*d++='%';*d++=hex[c>>4];*d++=hex[c&15];}}*d=0;}
static void nodeCopy(const Node*n,char*out,int cap){const char*l=strchr(n->host,':')?"[":"",*r=*l?"]":"";if(n->urlstyle||n->scheme==3){char*u=alloc(6144),*p=alloc(6144);encodeCredential(n->user,u);encodeCredential(n->pass,p);_snprintf(out,cap,"%s://%s:%s@%s%s%s:%s",schemes[n->scheme],u,p,l,n->host,r,n->port);drop(u);drop(p);}else _snprintf(out,cap,"%s://%s%s%s:%s:%s:%s",schemes[n->scheme],l,n->host,r,n->port,n->user,n->pass);}
static void start(){if(busy){busy=0;clearResult();lockform(0);put(CHECK,"检测代理");put(RAW,"已取消，旧请求结果不会显示。");return;}if(activeJobs>=3){put(RAW,"先前请求正在结束，请稍后重试。");return;}
 Job*j=alloc(sizeof(Job));Node*n=&j->n;n->scheme=SendMessageW(controls[PROTOCOL],0x147,0,0);n->v4=checked(V4);n->urlstyle=importedUrl;char*line=alloc(9000);readbox(PASTE,line,9000);changing=1;clearResult();
 if(*line){if(!parseNode(line,n)){put(RAW,"支持 主机:端口:账号:密码 或 socks5h://账号:密码@主机:端口。");goto bad;}importedUrl=n->urlstyle;put(HOST,n->host);put(PORT,n->port);put(USER,n->user);put(PASS,n->pass);put(PASTE,"");SendMessageW(controls[PROTOCOL],0x14e,n->scheme,0);}else{readbox(HOST,n->host,sizeof(n->host));readbox(PORT,n->port,sizeof(n->port));readbox(USER,n->user,sizeof(n->user));readbox(PASS,n->pass,sizeof(n->pass));}
 for(char*p=n->port;*p;p++)if(*p<'0'||*p>'9'){put(RAW,"端口必须为数字。");goto bad;}
 if(!*n->host||atoi(n->port)<1||atoi(n->port)>65535){put(RAW,"请填写有效主机和端口（1–65535）。");goto bad;}if(n->scheme>=2&&(strlen(n->user)>255||strlen(n->pass)>255)){put(RAW,"SOCKS5 账号或密码过长。");goto bad;}
 j->gen=generation;busy=1;lockform(1);put(CHECK,"取消检测");put(RAW,"正在通过代理检测出口 IP…");put(AUDIT,"等待连接结果。");__atomic_add_fetch(&activeJobs,1,__ATOMIC_SEQ_CST);H th=CreateThread(0,0,work,j,0,0);if(!th){__atomic_sub_fetch(&activeJobs,1,__ATOMIC_SEQ_CST);drop(j);busy=0;lockform(0);put(CHECK,"检测代理");put(RAW,"无法启动检测线程。");}else CloseHandle(th);drop(line);changing=0;return;
 bad:drop(line);drop(j);changing=0;}
static void renderJob(Job*j){char raw[1300];if(!j->connected){if(*j->error){put(RAW,j->error);put(AUDIT,"连接失败，未执行地区查询。");}return;}int primary=j->found[0]?0:j->found[1]?1:0;Geo*g=&j->geo[primary];
 if(j->geoReady)fmt(raw,"连接测试：成功\r\n出口 IP：%s\r\n州／城市：%s／%s\r\n经纬度：%s, %s",g->ip,val(g->state),val(g->city),val(g->lon),val(g->lat));else fmt(raw,"连接测试：成功\r\n出口 IP：%s\r\n正在查询两家定位资料…",g->ip);put(RAW,raw);
 nodeCopy(&j->n,clipNode,sizeof(clipNode));cp(clipCombined,clipNode);EnableWindow(controls[NODE],1);summary[0]=detailText[0]=clipRaw[0][0]=clipRaw[1][0]=0;EnableWindow(controls[ZIPRAW],0);EnableWindow(controls[ZIPRAW2],0);put(ZIPRAW,"复制接口邮编①");put(ZIPRAW2,"复制接口邮编②");twoGroups=j->found[0]&&j->found[1]&&!samePlace(&j->geo[0],&j->geo[1]);
 if(!j->geoReady)cp(summary,"等待两家定位结果。");else if(twoGroups){cp(summary,"IP 定位有差异\r\n\r\n");groupSummary(summary,sizeof(summary),&j->geo[0],&j->result[0],1,1);add(summary,"\r\n");groupSummary(summary,sizeof(summary),&j->geo[1],&j->result[1],1,2);}else{cp(summary,j->found[0]&&j->found[1]?"IP 州、城市一致\r\n\r\n":"定位对照未完成\r\n\r\n");if(j->found[primary])groupSummary(summary,sizeof(summary),g,&j->result[primary],0,primary+1);else add(summary,"两个定位渠道均未取得完整城市资料。");}
 for(int i=0;i<2;i++){char line[1000];Geo*x=&j->geo[i];fmt(line,"第%d组 %s：%s\r\n州／城市：%s／%s\r\n经纬度：%s, %s；接口原始邮编：%s（不用于复制住宅参考）\r\n",i+1,i?"IP2Location":"ipwho.is",j->found[i]?"已取得":"未取得完整资料",val(x->state),val(x->city),val(x->lon),val(x->lat),val(x->zip));add(detailText,line);clipRaw[i][0]=0;if(j->found[i]){const Lookup*r=resultFor(j,i);regionCopy(x,x->zip,r->area,1,clipRaw[i],sizeof(clipRaw[i]));}EnableWindow(controls[i?ZIPRAW2:ZIPRAW],*clipRaw[i]!=0);if(!j->found[i]){fmt(line,"定位请求状态 %u：%s\r\n",x->status,httpProblem(x->status));add(detailText,line);}add(detailText,j->result[i].details);add(detailText,"\r\n");}
 add(detailText,"住宅邮区参考依据公开已入住房屋记录，不代表代理实际住宅。邮区代表点不是具体地址。城市名称与邮政城市可能有差异。\r\n");
 cp(clipPlace[0],j->result[primary].copy);cp(clipPlace[1],twoGroups?j->result[1].copy:"");EnableWindow(controls[LOC],*clipPlace[0]!=0);EnableWindow(controls[LOC2],twoGroups&&*clipPlace[1]!=0);put(LOC,twoGroups?(*j->result[0].home?"复制第一组":"复制第一组城市"):(*j->result[primary].home?"复制城市邮编区号":"复制城市"));put(LOC2,*j->result[1].home?"复制第二组":"复制第二组城市");int agreed=j->found[0]&&j->found[1]&&samePlace(&j->geo[0],&j->geo[1]);combinedCopy(clipCombined,sizeof(clipCombined),clipNode,clipPlace[0],agreed);put(NODE,agreed?"复制节点＋地区（含密码）":"复制节点（含密码）");put(AUDIT,showDetails?detailText:summary);layout();}
static I proc(H w,U m,Z wp,I lp){if(m==1){mainwin=w;changing=1;font=CreateFontW(-15,0,0,0,400,0,0,0,1,0,0,5,0,ws("Microsoft YaHei UI"));const int labels[]={TITLE,LP,LPRO,LHOST,LUSER,LPASS,FOOT};const W*lt[]={ws("节点核查简易版"),ws("粘贴节点"),ws("协议"),ws("主机 / 端口"),ws("账号"),ws("密码"),ws("住宅邮编与电话区号自动查询 · 区域参考")};for(int i=0;i<7;i++)ctl(ws("STATIC"),lt[i],0,labels[i]);ctl(ws("BUTTON"),ws("关于"),0x10000,ABOUT);
 int boxes[]={PASTE,HOST,PORT,USER,PASS};for(int i=0;i<5;i++)ctl(ws("EDIT"),ws(""),0x10080|((i==0||i==4)?0x20:0),boxes[i]);put(PORT,"10001");ctl(ws("COMBOBOX"),ws(""),0x10003,PROTOCOL);const W*ps[]={ws("HTTP"),ws("HTTPS"),ws("SOCKS5"),ws("SOCKS5H")};for(int i=0;i<4;i++)SendMessageW(controls[PROTOCOL],0x143,0,(I)ps[i]);SendMessageW(controls[PROTOCOL],0x14e,2,0);
 ctl(ws("BUTTON"),ws("测 IPv4"),0x10003,V4);SendMessageW(controls[V4],0xf1,1,0);ctl(ws("BUTTON"),ws("窗口置顶"),0x10003,PIN);ctl(ws("BUTTON"),ws("显示"),0x10003,SHOW);ctl(ws("BUTTON"),ws("检测代理"),0x10001,CHECK);ctl(ws("BUTTON"),ws("复制节点（含密码）"),0x10000,NODE);
 ctl(ws("EDIT"),ws(""),0x210844,RAW);ctl(ws("EDIT"),ws(""),0x210844,AUDIT);SendMessageW(controls[AUDIT],0xc5,100000,0);ctl(ws("BUTTON"),ws("复制城市邮编区号"),0x10000,LOC);ctl(ws("BUTTON"),ws("复制第二组"),0x10000,LOC2);ctl(ws("BUTTON"),ws("详细信息"),0x10000,DETAIL);ctl(ws("BUTTON"),ws("复制接口邮编①"),0x10000,ZIPRAW);ctl(ws("BUTTON"),ws("复制接口邮编②"),0x10000,ZIPRAW2);clearResult();changing=0;return 0;}
 if(m==5&&controls[AUDIT]){layout();return 0;}if(m==0x24){int*z=(int*)lp;z[6]=430;z[7]=604;return 0;}
 if(m==0x111){int id=wp&65535,ev=(wp>>16)&65535;if(id==ABOUT&&!ev){MessageBoxW(w,ws("节点核查简易版 1.5.2 · Windows x64\n无需 AI 密钥。自动查询公开住宅邮区与电话区号。\nIP Geolocation by IP2Location.io\nwww.ip2location.io · ipwho.is\n本地住宅依据：美国人口普查局 ACS 2020–2024\n本地邮区及区号：zipcodes.pro（CC BY 4.0）\nzipcodes.pro/download\n邮编候选数据：GeoNames（CC BY 4.0）\nwww.geonames.org\n地图数据：OpenStreetMap contributors（ODbL）\n住宅与区号仅作区域参考，不能确定设备实际住址。"),ws("关于"),0x40);return 0;}if(id==CHECK&&!ev){start();return 0;}if(id==DETAIL&&!ev){showDetails=!showDetails;put(DETAIL,showDetails?"收起详情":"详细信息");put(AUDIT,showDetails?detailText:summary);return 0;}
 if(id==PIN&&!ev){SetWindowPos(w,(H)(I)(checked(PIN)?-1:-2),0,0,0,0,3);return 0;}if(id==SHOW&&!ev){SendMessageW(controls[PASS],0xcc,checked(SHOW)?0:0x25cf,0);extern int InvalidateRect(H,Rect*,int);InvalidateRect(controls[PASS],0,1);return 0;}
 if((id==NODE||id==LOC||id==LOC2||id==ZIPRAW||id==ZIPRAW2)&&!ev){int ok=clipboard(id==NODE?clipCombined:(id==ZIPRAW||id==ZIPRAW2)?clipRaw[id==ZIPRAW2]:clipPlace[id==LOC2]);put(id,ok?"已复制":"请重试复制");return 0;}
 if(!changing&&!busy&&((ev==0x300&&(id==PASTE||id==HOST||id==PORT||id==USER||id==PASS))||(id==V4&&!ev)||(id==PROTOCOL&&ev==1)))clearResult();return 0;}
 if(m==UPDATE||m==FINISH){Job*j=(Job*)lp;if(j->gen==generation){renderJob(j);if(m==FINISH){busy=0;lockform(0);put(CHECK,"检测代理");}}drop(j);return 0;}
 if(m==0x138&&(H)lp==controls[RAW]&&clipNode[0]){SetTextColor((H)wp,0x267334);SetBkColor((H)wp,0xffffff);return (I)GetSysColorBrush(5);}if(m==2){PostQuitMessage(0);return 0;}return DefWindowProcW(w,m,wp,lp);}
void entry(void){SetProcessDPIAware();inst=GetModuleHandleW(0);char wd[512];if(WSAStartup(0x202,wd))ExitProcess(1);Class c={0};c.proc=proc;c.inst=inst;c.icon=LoadIconW(inst,(const W*)101);c.cur=LoadCursorW(0,(const W*)32512);c.brush=(H)16;c.name=ws("NodeCheckMini152");RegisterClassW(&c);H w=CreateWindowExW(0,c.name,ws("节点核查简易版"),0x02cf0000,(int)0x80000000,0,550,744,0,0,inst,0);if(!w)ExitProcess(2);ShowWindow(w,5);UpdateWindow(w);SetFocus(controls[PASTE]);Msg m;while(GetMessageW(&m,0,0,0)>0){if(m.msg==0x100&&m.wp==13&&(m.hwnd==controls[PASTE]||m.hwnd==controls[PASS])){start();continue;}if(!IsDialogMessageW(w,&m)){TranslateMessage(&m);DispatchMessageW(&m);}}ExitProcess(0);}
