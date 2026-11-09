#include "fixture.c"
int main(void){
 CHECKIT(protocolTests()==0);
 Geo a={0};cp(a.country,"US");cp(a.state,"Florida");cp(a.city,"Orlando");cp(a.lat,"28.5383438");cp(a.lon,"-81.3792433");cp(a.zip,"32802");
 Lookup base={0};seedLocal(&a,&base);CHECKIT(base.homes==3&&hasCode(base.home,"32801"));CHECKIT(!hasCode(base.home,"32802")&&!hasCode(base.home,"32896"));CHECKIT(hasCode(base.area,"407")&&hasCode(base.area,"689")&&hasCode(base.area,"321"));CHECKIT(strstr(base.copy,"奥兰多")&&strstr(base.copy,"32801"));
 int failures[]={0,403,429,112002,112007,112175};
 for(int k=0;k<6;k++){Lookup l={0};volatile int gen=1;mockStatus=failures[k];mockResponse=0;lookupRegion(&a,&l,&gen,1,0,0);CHECKIT(l.complete&&same(l.home,base.home)&&same(l.area,base.area)&&same(l.copy,base.copy));}
 mockStatus=200;mockResponse="<html>Search verification required</html>";Lookup noLinks={0};volatile int gen=1;lookupRegion(&a,&noLinks,&gen,1,0,0);CHECKIT(same(noLinks.copy,base.copy));mockResponse=0;
 Geo unknown=a;cp(unknown.city,"Unknown City Example");Lookup city={0};mockStatus=403;lookupRegion(&unknown,&city,&gen,1,0,0);CHECKIT(!*city.home&&*city.copy&&strstr(city.copy,"Unknown City Example")&&!strstr(city.copy,"32802"));
 Geo atl=a,alpha=a;cp(atl.state,"Georgia");cp(atl.city,"Atlanta");cp(atl.lat,"33.7488");cp(atl.lon,"-84.388");alpha=atl;cp(alpha.city,"Alpharetta");cp(alpha.lat,"34.07562");cp(alpha.lon,"-84.29409");Lookup x={0},y={0};seedLocal(&atl,&x);seedLocal(&alpha,&y);CHECKIT(!samePlace(&atl,&alpha));CHECKIT(hasCode(x.home,"30303")&&!hasCode(x.home,"30334"));CHECKIT(!strstr(y.copy,"30303")&&!strstr(x.copy,"30009"));
 Geo boston=a;cp(boston.state,"Massachusetts");cp(boston.city,"Boston");cp(boston.lat,"42.3601");cp(boston.lon,"-71.0589");Lookup bo={0};seedLocal(&boston,&bo);CHECKIT(*bo.home&&bo.home[0]=='0'&&!hasCode(bo.home,"02133"));
 mockStatus=0;Lookup canceled={0};gen=2;int before=requestCount;lookupRegion(&a,&canceled,&gen,1,0,0);CHECKIT(requestCount==before&&*canceled.copy);
 Geo check={0};cp(check.ip,"8.8.8.8");mockStatus=200;mockResponse="{\"ip\":\"8.8.8.8\",\"success\":true,\"country_code\":\"US\",\"region\":\"California\",\"city\":\"San Jose\"}";CHECKIT(geolook(&check));cp(check.ip,"1.1.1.1");CHECKIT(!geolook(&check));
 puts("PASS 1.5: offline Orlando housing/area codes, 403/429/DNS/TLS/timeout/empty-search retention, city-only copy, separate cities, leading zeros, canceled requests, provider IP validation.");return 0;
}
