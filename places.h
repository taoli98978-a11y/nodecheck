static const char *stateNames[][2]={{"Alabama","阿拉巴马州"},{"Alaska","阿拉斯加州"},{"Arizona","亚利桑那州"},{"Arkansas","阿肯色州"},{"California","加利福尼亚州"},{"Colorado","科罗拉多州"},{"Connecticut","康涅狄格州"},{"Delaware","特拉华州"},{"Florida","佛罗里达州"},{"Georgia","佐治亚州"},{"Hawaii","夏威夷州"},{"Idaho","爱达荷州"},{"Illinois","伊利诺伊州"},{"Indiana","印第安纳州"},{"Iowa","艾奥瓦州"},{"Kansas","堪萨斯州"},{"Kentucky","肯塔基州"},{"Louisiana","路易斯安那州"},{"Maine","缅因州"},{"Maryland","马里兰州"},{"Massachusetts","马萨诸塞州"},{"Michigan","密歇根州"},{"Minnesota","明尼苏达州"},{"Mississippi","密西西比州"},{"Missouri","密苏里州"},{"Montana","蒙大拿州"},{"Nebraska","内布拉斯加州"},{"Nevada","内华达州"},{"New Hampshire","新罕布什尔州"},{"New Jersey","新泽西州"},{"New Mexico","新墨西哥州"},{"New York","纽约州"},{"North Carolina","北卡罗来纳州"},{"North Dakota","北达科他州"},{"Ohio","俄亥俄州"},{"Oklahoma","俄克拉何马州"},{"Oregon","俄勒冈州"},{"Pennsylvania","宾夕法尼亚州"},{"Rhode Island","罗得岛州"},{"South Carolina","南卡罗来纳州"},{"South Dakota","南达科他州"},{"Tennessee","田纳西州"},{"Texas","得克萨斯州"},{"Utah","犹他州"},{"Vermont","佛蒙特州"},{"Virginia","弗吉尼亚州"},{"Washington","华盛顿州"},{"West Virginia","西弗吉尼亚州"},{"Wisconsin","威斯康星州"},{"Wyoming","怀俄明州"},{"District of Columbia","哥伦比亚特区"}};
static const char *cityNames[][3]={{"Georgia","Alpharetta","阿尔法利塔"},{"Virginia","Virginia Beach","弗吉尼亚海滩"},{"Virginia","Richmond","里士满"},{"Virginia","Norfolk","诺福克"},{"Maryland","Baltimore","巴尔的摩"},{"South Carolina","Georgetown","乔治敦"},{"Georgia","Atlanta","亚特兰大"},{"Florida","Orlando","奥兰多"},{"Florida","Miami","迈阿密"},{"Florida","Tampa","坦帕"},{"Florida","Jacksonville","杰克逊维尔"},{"California","Los Angeles","洛杉矶"},{"California","San Francisco","旧金山"},{"California","San Diego","圣迭戈"},{"California","San Jose","圣何塞"},{"California","Sacramento","萨克拉门托"},{"California","Santa Monica","圣莫妮卡"},{"Texas","Austin","奥斯汀"},{"Texas","Dallas","达拉斯"},{"Texas","Houston","休斯敦"},{"Texas","San Antonio","圣安东尼奥"},{"Massachusetts","Boston","波士顿"},{"New York","New York","纽约市"},{"New York","New York City","纽约市"},{"Washington","Seattle","西雅图"},{"Illinois","Chicago","芝加哥"},{"Colorado","Denver","丹佛"},{"Nevada","Las Vegas","拉斯维加斯"},{"Oregon","Portland","波特兰"},{"Pennsylvania","Philadelphia","费城"},{"Pennsylvania","Pittsburgh","匹兹堡"},{"Arizona","Phoenix","菲尼克斯"},{"South Carolina","Charleston","查尔斯顿"},{"South Carolina","Columbia","哥伦比亚"},{"Michigan","Detroit","底特律"},{"District of Columbia","Washington","华盛顿"}};
static const char*stateZH(const char*country,const char*s){
 if(same(country,"US"))for(unsigned i=0;i<sizeof(stateNames)/sizeof(stateNames[0]);i++)if(same(s,stateNames[i][0]))return stateNames[i][1];
 return s;
}
static const char*cityZH(const char*country,const char*state,const char*city){
 if(same(country,"US")&&same(state,"Texas")&&same(city,"Fort Worth"))return "沃思堡";
 if(same(country,"US")&&same(state,"Alaska")&&same(city,"Anchorage"))return "安克雷奇";
 if(same(country,"US"))for(unsigned i=0;i<sizeof(cityNames)/sizeof(cityNames[0]);i++)if(same(state,cityNames[i][0])&&same(city,cityNames[i][1]))return cityNames[i][2];
 return city;
}
static void placeCopy(const Geo*g,char*out,int cap){
 const char*s=stateZH(g->country,g->state),*c=cityZH(g->country,g->state,g->city);
 _snprintf(out,cap-1,"%s%s%s %s",s,(*s&&((unsigned char)*s<128||(unsigned char)*c<128))?" ":"",c,g->zip);out[cap-1]=0;
}
static void difference(char*out,int cap,const char*key,const char*a,const char*b){
 if(!same(a,b)){if(*out)append(out,cap,"、");append(out,cap,key);if(!*a||!*b)append(out,cap,"缺项");}
}
