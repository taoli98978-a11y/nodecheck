#define main previousTests
#include "test150.c"
#undef main
static int parseInput(const char*s,Node*n){char input[9000];copyto(input,sizeof(input),s);return parseNode(input,n);}
int main(void){
 CHECKIT(previousTests()==0);
 Node n={0},m={0};char copy[13000];
 CHECKIT(parseInput("socks5h://user-session-1-sessionduration-1-country-us-zip-92646:p+word@gate.example:10000",&n));
 CHECKIT(n.scheme==3&&n.urlstyle&&!strcmp(n.host,"gate.example")&&!strcmp(n.port,"10000")&&!strcmp(n.pass,"p+word")&&!strcmp(n.user,"user-session-1-sessionduration-1-country-us-zip-92646"));
 nodeCopy(&n,copy,sizeof(copy));CHECKIT(parseInput(copy,&m)&&!strcmp(m.pass,n.pass)&&!strcmp(m.user,n.user)&&m.scheme==3);
 CHECKIT(parseInput("  SOCKS5H://u%40test:p%3A%40%25+%2B@[2001:db8::1]:10000\r\n",&n));
 CHECKIT(!strcmp(n.user,"u@test")&&!strcmp(n.pass,"p:@%++")&&!strcmp(n.host,"2001:db8::1"));nodeCopy(&n,copy,sizeof(copy));CHECKIT(parseInput(copy,&m)&&!strcmp(m.pass,n.pass)&&!strcmp(m.user,n.user));
 CHECKIT(parseInput("socks5://user:p:@word@host.example:10001",&n));CHECKIT(!strcmp(n.pass,"p:@word"));
 CHECKIT(parseInput("https://u:p@host.example:443",&n)&&n.scheme==1);
 CHECKIT(parseInput("http://host.example:8080",&n)&&n.scheme==0&&!*n.user);
 n.scheme=2;CHECKIT(parseInput("host.example:10001:u:p+:@x",&n)&&!n.urlstyle&&!strcmp(n.pass,"p+:@x"));nodeCopy(&n,copy,sizeof(copy));CHECKIT(!strcmp(copy,"socks5://host.example:10001:u:p+:@x"));
 CHECKIT(parseInput(copy,&m)&&!strcmp(m.pass,n.pass));
 CHECKIT(parseInput("socks5h://host.example:10001:u:p+",&n)&&n.scheme==3);nodeCopy(&n,copy,sizeof(copy));CHECKIT(parseInput(copy,&m)&&!strcmp(m.pass,"p+"));
 const char*bad[]={"socks6://u:p@host:80","socks5h://u:p@host:0","socks5h://u:p@host:65536","socks5h://u:p@host:abc","socks5h://u:p%00@host:80","socks5h://u:p%@host:80","socks5h://u:p%GG@host:80","socks5h://u:p@host:80/path","socks5h://u:p@[::1:80"};
 for(int i=0;i<sizeof(bad)/sizeof(bad[0]);i++)CHECKIT(!parseInput(bad[i],&n));
 puts("PASS 1.5.2: SOCKS5H / URI credentials / percent decoding / plus preservation / IPv6 / port validation / copy roundtrip. SOCKS transport regression verifies domain ATYP=3.");return 0;
}
