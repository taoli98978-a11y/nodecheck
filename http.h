#ifndef NODECHECK_TEST
extern H WinHttpOpen(const W*,U,const W*,const W*,U),WinHttpConnect(H,const W*,unsigned short,U),WinHttpOpenRequest(H,const W*,const W*,const W*,const W*,const W**,U);
extern int WinHttpSetTimeouts(H,int,int,int,int),WinHttpSendRequest(H,const W*,U,H,U,U,Z),WinHttpReceiveResponse(H,H),WinHttpQueryHeaders(H,U,const W*,H,U*,U*),WinHttpReadData(H,H,U,U*),WinHttpCloseHandle(H);
extern U GetLastError(void);
static char*requestOnce(const char*host,const char*path,const char*body,const char*headers,U*status){
 H session=WinHttpOpen(ws("NodeCheck/1.5 (interactive postal lookup)"),1,0,0,0),c=0,r=0;char*data=alloc(2097153);U len=0,code=0,sz=4,got=0;W*h=wide(host),*p=wide(path),*hdr=headers?wide(headers):0;
 if(session){WinHttpSetTimeouts(session,4500,4500,4500,6500);c=WinHttpConnect(session,h,443,0);}
 if(c)r=WinHttpOpenRequest(c,body?ws("POST"):ws("GET"),p,0,0,0,0x00800000|0x40); // HTTPS; path already percent-encoded
 if(r&&WinHttpSendRequest(r,hdr,hdr?(U)-1:0,(H)body,body?strlen(body):0,body?strlen(body):0,0)&&WinHttpReceiveResponse(r,0)&&WinHttpQueryHeaders(r,19|0x20000000,0,&code,&sz,0)){
  while(len<2097152){if(!WinHttpReadData(r,data+len,2097152-len,&got)){code=100000+GetLastError();len=0;break;}if(!got)break;len+=got;}if(len==2097152){code=200000;len=0;}data[len]=0;
 }else code=100000+GetLastError();
 if(r)WinHttpCloseHandle(r);if(c)WinHttpCloseHandle(c);if(session)WinHttpCloseHandle(session);drop(h);drop(p);drop(hdr);if(status)*status=code;return data;
}
static char*requestData(const char*h,const char*p,const char*b,const char*heads,U*status){U code=0;char*d=requestOnce(h,p,b,heads,&code);if(code==112002||code==112029||code==112030){drop(d);d=requestOnce(h,p,b,heads,&code);}if(status)*status=code;return d;}
static const char*httpProblem(U status){
 switch(status){case 112007:return "DNS 名称解析失败";case 112002:return "请求超时（已有限重试）";case 112029:return "无法连接服务器（已有限重试）";case 112030:return "连接中断（已有限重试）";case 112175:return "TLS 证书或安全连接失败";case 200000:return "响应过大，已停止读取";case 0:return "网络连接、DNS 或 TLS 失败/超时";case 403:return "服务拒绝访问";case 429:return "服务限流";case 404:return "查询页面不存在";case 200:return "响应中未找到符合条件的记录或字段";default:return "查询服务返回错误";}}
static char*get(const char*h,const char*p){return requestData(h,p,0,0,0);}
#else
static char*requestData(const char*,const char*,const char*,const char*,U*);
static char*get(const char*h,const char*p){return requestData(h,p,0,0,0);}
static const char*httpProblem(U s){return s==200?"页面字段不足":s==429?"服务限流":s==403?"服务拒绝访问":"网络或服务查询失败";}
#endif
