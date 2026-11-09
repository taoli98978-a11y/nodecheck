// Socket transport and nested TLS streams. Schannel validates the certificate and host name.
typedef uintptr_t Sock;
typedef struct AI {int flags,fam,type,proto; Z len; char*canon;void*addr;struct AI*next;} AI;
extern int WSAStartup(int,void*),getaddrinfo(const char*,const char*,const AI*,AI**),connect(Sock,const void*,int),closesocket(Sock),recv(Sock,char*,int,int),send(Sock,const char*,int,int),setsockopt(Sock,int,int,const char*,int);
extern void freeaddrinfo(AI*);extern Sock socket(int,int,int);
typedef struct {Z a,b;} Sec;
typedef struct {U len,kind;void*ptr;} Buf;
typedef struct {U version,count;Buf*buf;} Desc;
typedef struct {U version,count;void *certs,*store;U mappers;void*map;U algs;void*algo;U protocols,min,max,life,flags,format;} CredConfig;
extern int AcquireCredentialsHandleW(W*,const W*,U,H,H,H,H,Sec*,void*);
extern int InitializeSecurityContextW(Sec*,Sec*,W*,U,U,U,Desc*,U,Sec*,Desc*,U*,void*);
extern int FreeCredentialsHandle(Sec*),DeleteSecurityContext(Sec*),FreeContextBuffer(H),QueryContextAttributesW(Sec*,U,H),EncryptMessage(Sec*,U,Desc*,U),DecryptMessage(Sec*,Desc*,U,U*);
typedef struct Stream {struct Stream*below;Sock fd;Sec cred,ctx;int gotcred,gotctx;U hdr,tail,max,nbufs,block;unsigned char enc[75776],plain[65536];int nenc,nplain,start;} Stream;
static int wstream(Stream*,const void*,int);
static int rstream(Stream*s,void*dst,int limit){
 if(!s->below)return recv(s->fd,dst,limit,0);
 while(!s->nplain){
  if(s->nenc){
   Buf b[4]={{s->nenc,1,s->enc},{0},{0},{0}};Desc d={0,4,b};U qual=0;
   int e=DecryptMessage(&s->ctx,&d,0,&qual);
   if(e==0x90317)return 0;
   if(!e){
    int left=0;for(int i=0;i<4;i++){if(b[i].kind==1&&b[i].len){if(b[i].len>sizeof(s->plain))return -1;memcpy(s->plain,b[i].ptr,b[i].len);s->nplain=b[i].len;s->start=0;}if(b[i].kind==5)left=b[i].len;}
    if(left){for(int i=0;i<left;i++)s->enc[i]=s->enc[s->nenc-left+i];}s->nenc=left;if(s->nplain)break;continue;
   }
   if((U)e!=0x80090318)return -1;
  }
  if(s->nenc>=(int)sizeof(s->enc))return -1;
  int n=rstream(s->below,s->enc+s->nenc,sizeof(s->enc)-s->nenc);if(n<=0)return n;s->nenc+=n;
 }
 int n=s->nplain<limit?s->nplain:limit;memcpy(dst,s->plain+s->start,n);s->start+=n;s->nplain-=n;return n;
}
static int wstream(Stream*s,const void*data,int length){
 const char*p=data;if(!s->below){while(length){int n=send(s->fd,p,length,0);if(n<=0)return 0;length-=n;p+=n;}return 1;}
 while(length){int n=length<(int)s->max?length:(int)s->max;char*tmp=alloc(s->hdr+n+s->tail);if(!tmp)return 0;memcpy(tmp+s->hdr,p,n);
 Buf b[4]={{s->hdr,7,tmp},{n,1,tmp+s->hdr},{s->tail,6,tmp+s->hdr+n},{0}};Desc d={0,4,b};int ok=!EncryptMessage(&s->ctx,0,&d,0);
 for(int i=0;i<3&&ok;i++)ok=wstream(s->below,b[i].ptr,b[i].len);drop(tmp);if(!ok)return 0;p+=n;length-=n;}return 1;
}
static int secure(Stream*s,Stream*under,const char*hn){
 s->below=under;W*name=wide(hn);CredConfig c={0};c.version=4;c.flags=0x10|0x20; // no default client credentials; automatic server validation
 c.protocols=0x800; // TLS 1.2 client, supported by Windows 11
 long long expire=0;int ok=0;
 if(AcquireCredentialsHandleW(0,ws("Microsoft Unified Security Protocol Provider"),2,0,&c,0,0,&s->cred,&expire))goto done;
 s->gotcred=1;int first=1,extra=0;
 for(int i=0;i<180;i++){
  if(!first&&!extra){int n=rstream(under,s->enc+s->nenc,sizeof(s->enc)-s->nenc);if(n<=0)goto done;s->nenc+=n;}
  Buf in[2]={{s->nenc,2,s->enc},{0}},out={0,2,0};Desc id={0,2,in},od={0,1,&out};U attrs=0;
  int st=InitializeSecurityContextW(&s->cred,first?0:&s->ctx,name,0x811c,0,0,first?0:&id,0,&s->ctx,&od,&attrs,&expire);
  if(out.ptr){int sent=!out.len||wstream(under,out.ptr,out.len);FreeContextBuffer(out.ptr);if(!sent)goto done;}
  if((U)st==0x80090318){extra=0;continue;}
  if(st!=0&&st!=0x90312)goto done;s->gotctx=1;
  extra=in[1].kind==5?in[1].len:0;for(int k=0;k<extra;k++)s->enc[k]=s->enc[s->nenc-extra+k];s->nenc=extra;
  if(!st){ok=!QueryContextAttributesW(&s->ctx,4,&s->hdr);break;}first=0;
 }
 done:drop(name);return ok;
}
static void cleanup(Stream*s){if(s->gotctx)DeleteSecurityContext(&s->ctx);if(s->gotcred)FreeCredentialsHandle(&s->cred);}
static int exact(Stream*s,void*p,int count){char*b=p;while(count){int n=rstream(s,b,count);if(n<=0)return 0;b+=n;count-=n;}return 1;}
static int socks(Stream*s,const char*h,const char*u,const char*p){
 unsigned char pkt[800],r[4];int nu=strlen(u),np=strlen(p),nh=strlen(h);if(nu>255||np>255||nh>255)return 0;
 pkt[0]=5;pkt[1]=1;pkt[2]=nu||np?2:0;if(!wstream(s,pkt,3)||!exact(s,r,2)||r[0]!=5||r[1]!=pkt[2])return 0;
 if(pkt[2]){pkt[0]=1;pkt[1]=nu;memcpy(pkt+2,u,nu);pkt[2+nu]=np;memcpy(pkt+3+nu,p,np);if(!wstream(s,pkt,3+nu+np)||!exact(s,r,2)||r[0]!=1||r[1])return 0;}
 pkt[0]=5;pkt[1]=1;pkt[2]=0;pkt[3]=3;pkt[4]=nh;memcpy(pkt+5,h,nh);pkt[5+nh]=1;pkt[6+nh]=187;
 if(!wstream(s,pkt,nh+7)||!exact(s,r,4)||r[0]!=5||r[1])return 0;
 int n=r[3]==1?4:r[3]==4?16:0;if(r[3]==3){if(!exact(s,r,1))return 0;n=r[0];}return n&&exact(s,pkt,n+2);
}
static int httpTunnel(Stream*s,const char*h,const char*u,const char*p){
 char*msg=alloc(24000),*raw=alloc(9000),*encoded=alloc(13000);const char*alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";int ok=0;
 if(strlen(u)+strlen(p)>8000)goto done;
 _snprintf(raw,9000,"%s:%s",u,p);int n=strlen(raw),j=0;
 for(int i=0;i<n;i+=3){U x=(unsigned char)raw[i]<<16;if(i+1<n)x|=(unsigned char)raw[i+1]<<8;if(i+2<n)x|=(unsigned char)raw[i+2];encoded[j++]=alphabet[x>>18];encoded[j++]=alphabet[(x>>12)&63];encoded[j++]=i+1<n?alphabet[(x>>6)&63]:'=';encoded[j++]=i+2<n?alphabet[x&63]:'=';}
 if(*u||*p)_snprintf(msg,24000,"CONNECT %s:443 HTTP/1.1\r\nHost: %s:443\r\nProxy-Authorization: Basic %s\r\n\r\n",h,h,encoded);
 else _snprintf(msg,24000,"CONNECT %s:443 HTTP/1.1\r\nHost: %s:443\r\n\r\n",h,h);
 if(!wstream(s,msg,strlen(msg)))goto done;
 for(int i=0;i<18000;i++){if(!exact(s,msg+i,1))break;if(i>=3&&!strncmp(msg+i-3,"\r\n\r\n",4)){ok=!strncmp(msg,"HTTP/1.",7)&&msg[8]==' '&&atoi(msg+9)==200;break;}}
 done:drop(msg);drop(raw);drop(encoded);return ok;
}
extern int inet_pton(int,const char*,void*);
typedef struct {char host[260],port[8],user[2048],pass[2048];int scheme,v4,urlstyle;} Node;
static const char*probe(Node*n,char*ip,const char*dest){
 Stream*ss=alloc(3*sizeof(Stream)),*transport=ss;AI hint={0},*list=0;hint.type=1;hint.proto=6;Sock fd=(Sock)-1;const char*err="无法解析代理主机。";
 if(getaddrinfo(n->host,n->port,&hint,&list))goto end;
 err="无法连接代理端口。";for(AI*a=list;a;a=a->next){fd=socket(a->fam,1,6);if(fd==(Sock)-1)continue;int ms=10000;setsockopt(fd,65535,0x1006,(char*)&ms,4);setsockopt(fd,65535,0x1005,(char*)&ms,4);if(!connect(fd,a->addr,a->len))break;closesocket(fd);fd=(Sock)-1;}freeaddrinfo(list);if(fd==(Sock)-1)goto end;ss->fd=fd;
 if(n->scheme==1){err="HTTPS 代理 TLS 验证失败，请核对协议及证书。";if(!secure(ss+1,ss,n->host))goto end;transport=ss+1;}
 err="代理握手或认证失败，请核对协议、账号、密码和节点权限。";
 if(n->scheme>=2?!socks(transport,dest,n->user,n->pass):!httpTunnel(transport,dest,n->user,n->pass))goto end;
 err="代理已连接，出口查询的 TLS 验证失败。";if(!secure(ss+2,transport,dest))goto end;char req[240];fmt(req,"GET / HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",dest);
 err="出口查询未返回有效 IP。";if(!wstream(ss+2,req,strlen(req)))goto end;
 char*res=alloc(16385);int t=0;while(t<16384){int k=rstream(ss+2,res+t,16384-t);if(k<=0)break;t+=k;}char*b=strstr(res,"\r\n\r\n");
 if(b&&atoi(res+9)==200){b+=4;int l=0;while(b[l]&&b[l]!='\r'&&b[l]!='\n'&&b[l]!=' '&&l<63)l++;b[l]=0;unsigned char addr[16];if(inet_pton(2,b,addr)==1||(!n->v4&&inet_pton(23,b,addr)==1)){copyto(ip,64,b);err=0;}}drop(res);
 end:cleanup(ss+2);cleanup(ss+1);if(fd!=(Sock)-1)closesocket(fd);drop(ss);return err;
}
